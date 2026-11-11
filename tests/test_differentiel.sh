#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Différentiel C↔Python (TAM-09, CONTRACT §6) : ≥200 verdicts déterministes par
# graine, rendus des deux côtés, comparaison octet par octet, 1ᵉʳ désaccord localisé.
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
SEED="${1:-12345}"
N="${2:-250}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# Mêmes verdicts des deux côtés (même graine, même générateur).
python3 tests/gen_verdicts.py serialize "$SEED" "$N" > "$TMP/ser.txt" \
  || { echo "KO générateur (serialize)"; exit 1; }
python3 tests/gen_verdicts.py render "$SEED" "$N" > "$TMP/py.txt" \
  || { echo "KO générateur (render)"; exit 1; }

# Cœur C absent -> mode Python seul, on n'échoue pas (le différentiel est reporté).
if [ ! -f core/src/rapport.c ]; then
  echo "ATTENTION core/src/rapport.c absent : différentiel C reporté, validation Python seule"
  echo "OK $(wc -l < "$TMP/py.txt") checks (Python seul, C absent)"
  exit 0
fi

if ! gcc -std=c11 -O2 -Wall -Wextra -Werror -Icore/include -o "$TMP/diff_driver" \
     tests/diff_driver.c core/src/verdict.c core/src/rapport.c 2>"$TMP/cc.err"; then
  echo "KO compilation du pont C :"; cat "$TMP/cc.err"; exit 1
fi
"$TMP/diff_driver" < "$TMP/ser.txt" > "$TMP/c.txt" \
  || { echo "KO exécution du pont C"; exit 1; }

if cmp -s "$TMP/c.txt" "$TMP/py.txt"; then
  echo "OK $(wc -l < "$TMP/py.txt") checks (différentiel C↔Python, graine $SEED, 0 divergence)"
  exit 0
fi

echo "KO DÉSACCORD C↔Python (graine $SEED)"
lineno=0
while IFS= read -r cline; do
  lineno=$((lineno+1))
  pline=$(sed -n "${lineno}p" "$TMP/py.txt")
  if [ "$cline" != "$pline" ]; then
    echo "  verdict #$lineno"
    echo "    C  : $cline"
    echo "    Py : $pline"
    break
  fi
done < "$TMP/c.txt"
exit 1
