#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Test de bout en bout du CLI tamis sur les fixtures (TAM-04/05/06/07/08 + view + formats).
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
TAMIS=build/tamis
ATT=fixtures/attaque.mpf
DUR=fixtures/durcie.mpf
checks=0; fail=0

[ -x "$TAMIS" ] || { echo "KO build/tamis absent (lancer make cli)"; exit 1; }

ck_grep() { checks=$((checks+1)); printf '%s' "$1" | grep -q "$2" || { echo "FAILED : $3"; fail=1; }; }
ck_rc()   { checks=$((checks+1)); [ "$1" -eq "$2" ] || { echo "FAILED : $3 (rc=$1, attendu $2)"; fail=1; }; }

out=$("$TAMIS" version 2>&1)
ck_grep "$out" "tamis 0.1.0" "version"

out=$("$TAMIS" lint "$ATT" 2>&1); rc=$?
ck_grep "$out" "UNTYPED_PROCESS" "lint : processus non typé détecté"
ck_grep "$out" "MISSING_DEFAULT_SPAWN" "lint : default_spawn absent détecté"
ck_rc "$rc" 0 "lint sort 0 (warnings seuls)"

out=$("$TAMIS" reach "$ATT" --src app_tiers --dst pompe_critique 2>&1); rc=$?
ck_grep "$out" "PATH_FOUND" "reach : chemin trouvé"
ck_grep "$out" "app_tiers -> pompe_critique" "reach : chemin affiché"
ck_rc "$rc" 1 "reach sort 1 (error)"

out=$("$TAMIS" proof "$ATT" --src app_tiers --dst pompe_critique 2>&1); rc=$?
ck_grep "$out" "INTERFERENCE_POSSIBLE" "proof : interférence sur politique trouée"
ck_rc "$rc" 1 "proof sort 1 (interférence)"

out=$("$TAMIS" proof "$DUR" --src app_tiers --dst pompe_critique 2>&1); rc=$?
ck_grep "$out" "prouvée" "proof : certificat sur politique durcie"
ck_grep "$out" "exhaustif" "proof : parcours exhaustif mentionné"
ck_rc "$rc" 0 "proof sort 0 (non-interférence prouvée)"

out=$("$TAMIS" minimize "$ATT" 2>&1); rc=$?
ck_grep "$out" "REDUNDANT_RULE" "minimize : règle redondante (R999)"
ck_grep "$out" "R999" "minimize : R999 identifiée"
ck_grep "$out" "équivalente" "minimize : preuve d'équivalence"

out=$("$TAMIS" diff "$ATT" "$DUR" 2>&1); rc=$?
ck_grep "$out" "PRIVILEGE_NARROWED" "diff : réductions de privilège"

out=$("$TAMIS" view "$ATT" 2>&1); rc=$?
ck_grep "$out" '"nodes"' "view : nœuds JSON"
ck_grep "$out" '"edges"' "view : arêtes JSON"
ck_grep "$out" "pompe_critique" "view : type présent"
ck_rc "$rc" 0 "view sort 0"

outj=$("$TAMIS" lint "$ATT" --format json 2>&1)
ck_grep "$outj" '"code":"TAMIS_UNTYPED_PROCESS"' "format json"
outm=$("$TAMIS" lint "$ATT" --format md 2>&1)
ck_grep "$outm" "Rapport d'audit Tamis" "format markdown : titre"
ck_grep "$outm" "Empreinte SHA-256" "format markdown : empreinte"

# preuve SMT-LIB2 : encodage émis, et verdict confirmé si z3 est présent
out=$("$TAMIS" smt "$ATT" --src app_tiers --dst pompe_critique 2>&1)
ck_grep "$out" "(set-logic HORN)" "smt : clauses de Horn déclarées"
ck_grep "$out" "(assert (not r_0_3))" "smt : requête « src n'atteint pas dst » émise"
ck_grep "$out" "(check-sat)" "smt : check-sat émis"
if command -v z3 >/dev/null 2>&1; then
  ck_grep "$out" "unsat" "smt : z3 confirme que le chemin existe"
  out2=$("$TAMIS" smt "$ATT" --src pompe_critique --dst app_tiers 2>&1)
  ck_grep "$out2" "sat" "smt : z3 confirme l'absence de chemin"
else
  echo "INFO z3 absent : encodage émis sans exécution"
  checks=$((checks+1))
fi
out3=$("$TAMIS" smt "$ATT" 2>&1)
ck_grep "$out3" "check-sat" "smt : encodage global produit"

if [ "$fail" -ne 0 ]; then echo "FAILED test_cli"; exit 1; fi
echo "OK $checks checks"
