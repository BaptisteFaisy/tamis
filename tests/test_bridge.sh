#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Test du pont local : comportement + propriétés de sécurité (jeton, Origin, liste blanche,
# écoute locale uniquement). Démarre le pont sur un port de test puis l'arrête toujours.
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
[ -x build/tamis ] || make -s cli >/dev/null 2>&1

PORT="${1:-8791}"
JETON="jeton-de-test"
BASE="http://127.0.0.1:$PORT"
checks=0; fail=0

ck() { checks=$((checks+1)); if [ "$1" != "$2" ]; then echo "KO   $3 (obtenu [$1], attendu [$2])"; fail=1; else echo "OK   $3"; fi; }
ckc() { checks=$((checks+1)); if printf '%s' "$1" | grep -qF "$2"; then echo "OK   $3"; else echo "KO   $3 (absent de la sortie)"; fail=1; fi; }

python3 tools/tamis-bridge.py --port "$PORT" --token-fixe "$JETON" >/tmp/bridge-$PORT.log 2>&1 &
PID=$!
trap 'kill $PID 2>/dev/null; wait $PID 2>/dev/null' EXIT

# attente bornée du démarrage
for _ in $(seq 1 40); do
  curl -s --max-time 1 "$BASE/ping" >/dev/null 2>&1 && break
  sleep 0.25
done

# 1. /ping répond sans jeton (détection de présence) et ne fuit rien
code=$(curl -s -o /tmp/ping.json -w '%{http_code}' --max-time 5 "$BASE/ping")
ck "$code" "200" "ping accessible sans jeton"
ckc "$(cat /tmp/ping.json)" '"service":"tamis-bridge"' "ping annonce le service"
ckc "$(cat /tmp/ping.json)" '"cli_c"' "ping dit si le cœur C est construit"

# 2. sans jeton : refus 401
code=$(curl -s -o /dev/null -w '%{http_code}' --max-time 5 -X POST "$BASE/tamis" -H 'Content-Type: application/json' -d '{"cmd":"lint","policy":"type a"}')
ck "$code" "401" "POST /tamis sans jeton refusé (401)"

# 3. mauvais jeton : refus 401
code=$(curl -s -o /dev/null -w '%{http_code}' --max-time 5 -X POST "$BASE/tamis" -H 'Content-Type: application/json' -H 'X-Tamis-Token: faux' -d '{"cmd":"lint","policy":"type a"}')
ck "$code" "401" "POST /tamis avec un mauvais jeton refusé (401)"

# 4. Origine non admise : refus 403
code=$(curl -s -o /dev/null -w '%{http_code}' --max-time 5 "$BASE/presets" -H "Origin: https://malveillant.example" -H "X-Tamis-Token: $JETON")
ck "$code" "403" "Origine tierce refusée (403)"

# 5. le vrai cœur C répond, et comme le CLI en direct
out=$(curl -s --max-time 20 -X POST "$BASE/tamis" -H 'Content-Type: application/json' -H "X-Tamis-Token: $JETON" \
  -d '{"cmd":"lint","policy":"type a\ntype b\nallow a b msg @R1\nuntyped /bin/x\n","format":"text"}')
ckc "$out" 'TAMIS_UNTYPED_PROCESS' "pont → cœur C : lint détecte le processus non typé"
ckc "$out" 'TAMIS_MISSING_DEFAULT_SPAWN' "pont → cœur C : default_spawn absent"
ckc "$out" '"engine":"core C' "le pont annonce provenir du cœur C"

# 6. le pont et le CLI en direct doivent dire la même chose
printf 'type a\ntype b\nallow a b msg @R1\nuntyped /bin/x\n' > /tmp/pont-test.mpf
direct=$(./build/tamis lint /tmp/pont-test.mpf --label policy.mpf --format json 2>/dev/null)
via=$(curl -s --max-time 20 -X POST "$BASE/tamis" -H 'Content-Type: application/json' -H "X-Tamis-Token: $JETON" \
  -d '{"cmd":"lint","policy":"type a\ntype b\nallow a b msg @R1\nuntyped /bin/x\n","format":"json"}' \
  | python3 -c 'import json,sys; print(json.load(sys.stdin)["stdout"].strip())')
ck "$via" "$direct" "pont == CLI direct (JSON identique)"

# 7. commande hors liste blanche refusée
out=$(curl -s --max-time 10 -X POST "$BASE/tamis" -H 'Content-Type: application/json' -H "X-Tamis-Token: $JETON" \
  -d '{"cmd":"rm -rf /","policy":"type a"}')
ckc "$out" 'commande refusée' "commande hors liste blanche refusée"

# 8. src/dst non conformes refusés (pas d'injection)
out=$(curl -s --max-time 10 -X POST "$BASE/tamis" -H 'Content-Type: application/json' -H "X-Tamis-Token: $JETON" \
  -d '{"cmd":"reach","policy":"type a\ntype b\nallow a b\n","src":"a; rm -rf /","dst":"b"}')
ckc "$out" 'invalide' "src contenant des métacaractères refusé"

# 9. préréglages QNX listés, et aucun préréglage arbitraire accepté
ckc "$(curl -s --max-time 5 "$BASE/presets" -H "X-Tamis-Token: $JETON")" '"uname"' "liste des préréglages QNX"
out=$(curl -s --max-time 10 -X POST "$BASE/qnx/exec" -H 'Content-Type: application/json' -H "X-Tamis-Token: $JETON" -d '{"preset":"reboot"}')
ckc "$out" 'refusé' "préréglage inconnu refusé"

# 9b. l'état du banc est rapporté (local, distant ou non configuré) — jamais de plantage
ckc "$(curl -s --max-time 20 "$BASE/qnx/status" -H "X-Tamis-Token: $JETON")" '"banc"' "état du banc QNX rapporté"

# 9c. banc distant injoignable : dégradation propre, pas d'exception
out=$(TAMIS_QNX_HOST=hote-inexistant.invalid TAMIS_QNX_DIR='~/x' timeout 60 python3 -c \
  'import sys; sys.path.insert(0,"tools"); import bridge_backend as B; print(B.qnx_status().get("running"))' 2>&1 | tail -1)
ck "$out" "False" "banc distant injoignable : dégradation propre"

# 10. n'écoute PAS sur les interfaces réseau
IP=$(hostname -I 2>/dev/null | awk '{print $1}')
if [ -n "${IP:-}" ] && [ "$IP" != "127.0.0.1" ]; then
  if curl -s --max-time 3 "http://$IP:$PORT/ping" >/dev/null 2>&1; then
    echo "KO   le pont répond sur l'interface réseau $IP"; fail=1; checks=$((checks+1))
  else
    echo "OK   injoignable depuis l'interface réseau $IP (127.0.0.1 seulement)"; checks=$((checks+1))
  fi
fi

rm -f /tmp/pont-test.mpf /tmp/ping.json
[ "$fail" -ne 0 ] && { echo "FAILED test_bridge"; exit 1; }
echo "OK $checks checks"
