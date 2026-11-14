#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Prouve que la vitrine Tamis et le service de preuves sont joignables DEPUIS
# INTERNET — par l'entrée publique de Tailscale Funnel, pas par le seul tailnet.
#
# Pourquoi ce script : Funnel ne publie QUE les ports 443, 8443 et 10000
# (https://tailscale.com/kb/1223/funnel). Un service monté sur un autre port
# (:8092, :8094…) reste joignable depuis le tailnet : un juge hors du tailnet y
# voit une connexion qui expire, alors que `tailscale serve status` annonce
# « Funnel on ». Le test résout donc le nom par un résolveur PUBLIC et épingle
# la requête sur cette IP (--resolve) : le trafic sort sur internet et revient
# par l'entrée Funnel, ce qui exclut tout routage tailnet (100.64/10).
#
# Usage : banc/prouve-public.sh [--nom FQDN] [--resolver IP] [--port N]
#                              [--etapes "id id …"] [--rapport FICHIER]
# Sortie : 0 = tout passe ; 1 = au moins un échec ; 2 = option inconnue ;
#          3 = prérequis absent (curl, dig, résolution).
set -uo pipefail

DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
# shellcheck source=./ck.sh
source "$DIR/ck.sh"

NOM="${TAMIS_VITRINE_NOM:-r630-debian.tail3a8bdf.ts.net}"
RESOLVEUR="${TAMIS_VITRINE_RESOLVEUR:-1.1.1.1}"
PORT="${TAMIS_VITRINE_PORT:-8443}"
MONTAGE_VITRINE="${TAMIS_VITRINE_MONTAGE:-/tamis}"
MONTAGE_API="${TAMIS_API_MONTAGE:-/tamis-api}"
ETAPES="${TAMIS_ETAPES:-version lint reach explain proof smt}"
RAPPORT=""

while [ $# -gt 0 ]; do
  case "$1" in
    --nom) NOM="${2:?}"; shift 2 ;;
    --resolver) RESOLVEUR="${2:?}"; shift 2 ;;
    --port) PORT="${2:?}"; shift 2 ;;
    --etapes) ETAPES="${2:?}"; shift 2 ;;
    --rapport) RAPPORT="${2:?}"; shift 2 ;;
    --help|-h) sed -n '3,17p' "$0"; exit 0 ;;
    *) echo "KO option inconnue : $1" >&2; exit 2 ;;
  esac
done

command -v curl >/dev/null || { echo "KO curl absent" >&2; exit 3; }
command -v dig >/dev/null || { echo "KO dig absent (paquet dnsutils)" >&2; exit 3; }

IP=$(dig +short @"$RESOLVEUR" "$NOM" A | grep -E '^[0-9.]+$' | head -1)
if [ -z "$IP" ]; then
  echo "KO nom non résolu par le résolveur public $RESOLVEUR : $NOM" >&2
  exit 3
fi

echo "── vitrine publique Tamis — $NOM ──"
echo "entrée Funnel (via $RESOLVEUR) : $IP"
echo "route locale                  : $(ip route get "$IP" 2>/dev/null | head -1 || echo '(n/d)')"

get() {  # get <chemin> <fichier> → "code|type"
  curl -s -m 60 --resolve "$NOM:$PORT:$IP" -o "$2" \
    -w '%{http_code}|%{content_type}' "https://$NOM:$PORT$1"
}

verif_vitrine() {  # la page de l'invite : écran VGA + console série
  local reponse code
  reponse=$(get "$MONTAGE_VITRINE/" /tmp/tamis-vitrine.html); code="${reponse%%|*}"
  ck "$([ "$code" = 200 ] && echo 0 || echo 1)" "GET $MONTAGE_VITRINE/ → HTTP $code"
  grep -q 'id="screen"' /tmp/tamis-vitrine.html
  ck $? "la page embarque l'écran de l'invite QNX 8.0"
  grep -q 'var B=' /tmp/tamis-vitrine.html
  ck $? "les URLs des assets suivent le montage (base dérivée en JS)"
  reponse=$(get "$MONTAGE_VITRINE/age" /tmp/tamis-age)
  code="${reponse%%|*}"
  ck "$([ "$code" = 200 ] && echo 0 || echo 1)" "GET $MONTAGE_VITRINE/age → HTTP $code ($(wc -c </tmp/tamis-age) octets)"
}

verif_flux() {  # le flux JPEG en continu (multipart) : le juge voit la cible bouger
  local code type
  code=$(curl -s -m 12 --resolve "$NOM:$PORT:$IP" -o /tmp/tamis-stream \
    -w '%{http_code}|%{content_type}' "https://$NOM:$PORT$MONTAGE_VITRINE/stream")
  type="${code#*|}"; code="${code%%|*}"
  ck "$([ "$code" = 200 ] && echo 0 || echo 1)" "GET $MONTAGE_VITRINE/stream → HTTP $code"
  case "$type" in *mixed-replace*) ck 0 "flux multipart JPEG ($type)";; *) ck 1 "flux attendu en multipart, eu « $type »";; esac
  ck "$([ "$(wc -c </tmp/tamis-stream)" -gt 10000 ] && echo 0 || echo 1)" "le flux transporte des images ($(wc -c </tmp/tamis-stream) octets en 12 s)"
}

verif_api() {  # état du service + liste fermée d'étapes
  local reponse code
  reponse=$(get "$MONTAGE_API/etat" /tmp/tamis-etat.json); code="${reponse%%|*}"
  ck "$([ "$code" = 200 ] && echo 0 || echo 1)" "GET $MONTAGE_API/etat → HTTP $code"
  python3 -c "
import json,sys
d = json.load(open('/tmp/tamis-etat.json'))
ids = [p['id'] for p in d['preuves']]
attendues = sys.argv[1:]
manquantes = [i for i in attendues if i not in ids]
sys.exit(1 if (manquantes or not d['invite'].get('pret')) else 0)
" $ETAPES
  ck $? "l'état annonce l'invité prêt et toutes les étapes ($ETAPES)"
}

verif_etapes() {  # chaque étape, exécutée DANS l'invité, via le montage public
  local reponse code id
  for id in $ETAPES; do
    reponse=$(get "$MONTAGE_API/preuve/$id" "/tmp/tamis-preuve-$id.json"); code="${reponse%%|*}"
    ck "$([ "$code" = 200 ] && echo 0 || echo 1)" "GET $MONTAGE_API/preuve/$id → HTTP $code"
    python3 - "$id" <<'PY'
import json, sys
pid = sys.argv[1]
d = json.load(open(f"/tmp/tamis-preuve-{pid}.json"))
sortie = (d.get("sortie_invite") or "").strip()
comparaison = d.get("identique_octet_pour_octet")
ok = sortie and (comparaison is not False)
sys.exit(0 if ok else 1)
PY
    ck $? "$id : sortie réelle dans l'invité, comparaison hôte↔invité sans échec"
  done
}

verif_port_non_publie() {  # rappel du piège : un port hors 443/8443/10000 n'est PAS public
  local code
  code=$(curl -s -m 12 --resolve "$NOM:8094:$IP" -o /dev/null -w '%{http_code}' \
    "https://$NOM:8094/etat" 2>/dev/null)
  ck "$([ "$code" = 200 ] && echo 1 || echo 0)" ":8094 (hors liste Funnel) reste bien injoignable depuis internet (HTTP ${code:-aucun})"
}

verif_vitrine
verif_flux
verif_api
verif_etapes
verif_port_non_publie

if [ -n "$RAPPORT" ]; then
  { echo "# Vitrine publique — $NOM (port $PORT, montages $MONTAGE_VITRINE et $MONTAGE_API)"
    echo "- date : $(date -Is) · entrée Funnel : $IP (résolue via $RESOLVEUR)"
    echo "- exécuté par : $(whoami)@$(hostname) · issue de : banc/prouve-public.sh"
    echo "- résultat : $checks vérifications, $fail échec(s)"
    echo "- étapes : $ETAPES"
  } > "$RAPPORT"
fi

ck_final "($NOM:$PORT)"
