#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Différentiel JS↔C : le jumeau navigateur (site/tamis-*.js) doit rendre EXACTEMENT comme
# le cœur C (build/tamis). Même commande, même JSON, octet pour octet (CONTRACT §5.2).
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"

if ! command -v node >/dev/null 2>&1; then
  echo "ATTENTION node absent : différentiel JS↔C ignoré (le site reste fonctionnel dans un navigateur)"
  echo "OK 0 checks"
  exit 0
fi
[ -x build/tamis ] || make -s cli >/dev/null 2>&1 || { echo "KO build/tamis absent"; exit 1; }
node --check site/tamis-core.js >/dev/null || { echo "KO syntaxe site/tamis-core.js"; exit 1; }
node --check site/tamis-render.js >/dev/null || { echo "KO syntaxe site/tamis-render.js"; exit 1; }

ATT=fixtures/attaque.mpf
DUR=fixtures/durcie.mpf
checks=0; fail=0

# run_pair <description> <cmd> <fichier> <fichier2|-> <src|-> <dst|->
run_pair() {
  local desc="$1" cmd="$2" f1="$3" f2="$4" s="$5" d="$6"
  local -a c_args=("$cmd" "$f1") j_args=("$cmd" "$f1" "")
  if [ "$f2" != "-" ]; then c_args+=("$f2"); j_args=("$cmd" "$f1" "$f2"); fi
  if [ "$s" != "-" ]; then c_args+=(--src "$s" --dst "$d"); j_args+=("$s" "$d"); fi
  c_args+=(--format json)
  local c_out js_out
  c_out=$(./build/tamis "${c_args[@]}" 2>/dev/null || true)
  js_out=$(node tests/js_diff_driver.js "${j_args[@]}" 2>/dev/null || true)
  checks=$((checks + 1))
  if [ "$c_out" = "$js_out" ]; then
    printf 'OK   %s\n' "$desc"
  else
    printf 'KO   %s : C ≠ JS\n' "$desc"
    diff <(printf '%s\n' "$c_out") <(printf '%s\n' "$js_out") | head -8 | sed 's/^/     /'
    fail=1
  fi
}

run_pair "lint sur attaque"            lint     "$ATT" - - -
run_pair "lint sur durcie"             lint     "$DUR" - - -
run_pair "minimize sur attaque"        minimize "$ATT" - - -
run_pair "minimize sur durcie"         minimize "$DUR" - - -
run_pair "reach chemin trouvé"         reach    "$ATT" - app_tiers pompe_critique
run_pair "reach chemin coupé"          reach    "$DUR" - app_tiers pompe_critique
run_pair "reach sens inverse"          reach    "$ATT" - pompe_critique app_tiers
run_pair "reach autre source"          reach    "$ATT" - reseau pompe_critique
run_pair "proof interférence"          proof    "$ATT" - app_tiers pompe_critique
run_pair "proof certificat"            proof    "$DUR" - app_tiers pompe_critique
run_pair "explain chemin + correctif"  explain  "$ATT" - app_tiers pompe_critique
run_pair "explain sain + angles morts" explain  "$DUR" - app_tiers pompe_critique
run_pair "diff attaque → durcie"       diff     "$ATT" "$DUR" - -

# ——— vrai langage secpol (QNX 8.0) : le jumeau JS doit rendre comme le cœur C ———
QNX8=fixtures/politique-qnx8.secpol
if [ -f "$QNX8" ]; then
  run_pair "secpol lint politique QNX 8.0"      lint    "$QNX8" - - -
  run_pair "secpol reach app_tiers→pompe"       reach   "$QNX8" - app_tiers pompe_critique
  run_pair "secpol proof certificat"            proof   "$QNX8" - app_tiers pompe_critique
  run_pair "secpol explain"                     explain "$QNX8" - app_tiers pompe_critique
  run_pair "secpol reach mm_client→mm_server"   reach   "$QNX8" - mm_client mm_server
  run_pair "secpol minimize"                    minimize "$QNX8" - - -
fi
# Corpus oracle : 42 politiques (25 invalides, 17 valides) — refus et positions identiques.
SECPOL_N=0
for f in fixtures/secpol-cas/*.secpol; do
  [ -f "$f" ] || continue
  run_pair "secpol corpus $(basename "$f")" lint "$f" - - -
  SECPOL_N=$((SECPOL_N + 1))
done

if [ "$fail" -ne 0 ]; then
  echo "FAILED différentiel JS↔C"
  exit 1
fi
echo "OK $checks checks (jumeau navigateur identique au cœur C, dont $SECPOL_N cas secpol)"
