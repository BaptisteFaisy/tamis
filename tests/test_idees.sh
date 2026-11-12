#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Test du catalogue d'idées : fusion sans doublon, numérotation continue, 10 thèmes.
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
checks=0; fail=0
ck() { checks=$((checks+1)); if [ "$1" -eq 0 ]; then echo "OK   $2"; else echo "KO   $2"; fail=1; fi; }
ckc() { checks=$((checks+1)); if grep -qF "$2" <<< "$1"; then echo "OK   $3"; else echo "KO   $3 (absent)"; fail=1; fi; }

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

out=$(python3 tools/merge_idees.py --racine docs/idees 2>&1)
ck $? "merge_idees.py s'exécute"
ckc "$out" "1000 idées uniques" "le catalogue compte 1000 idées uniques"
ckc "$out" "10 thèmes" "le catalogue compte 10 thèmes"

F="$RACINE/docs/1000-idees-consolidation.md"
[ -f "$F" ]; ck $? "docs/1000-idees-consolidation.md existe"
n=$(grep -cE '^[0-9]+\. ' "$F")
[ "$n" -eq 1000 ]; ck $? "1000 lignes d'idées dans le catalogue (trouvé $n)"
dups=$(grep -E '^[0-9]+\. ' "$F" | sed 's/^[0-9]*\. //' | tr 'A-Z' 'a-z' | sort | uniq -d | wc -l)
[ "$dups" -eq 0 ]; ck $? "aucun doublon exact"
nums=$(grep -oE '^[0-9]+\. ' "$F" | tr -d '. ' | sort -n | uniq | wc -l)
[ "$nums" -eq 1000 ]; ck $? "numérotation continue de 1 à 1000"
themes=$(grep -cE '^## ' "$F")
[ "$themes" -ge 10 ]; ck $? "les thèmes sont titrés dans le catalogue"
[ -f "$RACINE/docs/1000-idees-consolidation.json" ]; ck $? "version JSON présente"
[ -f "$RACINE/docs/top10-consolidation.md" ]; ck $? "la sélection des 10 meilleures est versionnée"

if [ "$fail" -ne 0 ]; then
  echo "FAILED test_idees"
  exit 1
fi
echo "OK $checks checks"