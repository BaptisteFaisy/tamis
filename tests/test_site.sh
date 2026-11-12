#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Site de démonstration : (1) câblage vérifié en exécutant app.js sur un mini-DOM,
# (2) absence de dérive entre site/ et ses sources (fixtures/, docs/).
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"

if ! command -v node >/dev/null 2>&1; then
  echo "ATTENTION node absent : smoke test du site ignoré"
  echo "OK 0 checks"
  exit 0
fi

for f in site/tamis-core.js site/tamis-render.js site/app.js; do
  node --check "$f" >/dev/null || { echo "KO syntaxe $f"; exit 1; }
done

if ! out=$(timeout 120 node tests/site_smoke.js 2>&1); then
  echo "KO smoke test du site :"
  printf '%s\n' "$out" | tail -15
  exit 1
fi
printf '%s\n' "$out" | grep -E '^(OK|FAILED)' | tail -2

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
python3 tools/build_site.py --out "$TMP/site" >/dev/null \
  || { echo "KO régénération du site"; exit 1; }
if ! diff -r site "$TMP/site" >/dev/null 2>&1; then
  echo "KO site/ n'est pas à jour — relancer : python3 tools/build_site.py"
  diff -r site "$TMP/site" | head -10
  exit 1
fi
echo "OK site/ conforme à ses sources (aucune dérive)"
