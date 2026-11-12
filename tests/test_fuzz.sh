#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Fuzz du parseur + moteurs sous ASan/UBSan (TAM-11). Campagne bornée pour la CI ;
# la campagne longue (200 000 entrées) est un run manuel/nocturne : `tools/fuzz.c <graine> <n>`.
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
BIN="/tmp/tamis_fuzz_$$"
CCERR="/tmp/tamis_fuzz_cc_$$"

if ! gcc -std=c11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
     -Icore/include -o "$BIN" tools/fuzz.c core/src/*.c 2>"$CCERR"; then
  echo "KO compilation du fuzzer :"; cat "$CCERR"; rm -f "$CCERR"; exit 1
fi
rm -f "$CCERR"

total=0
for seed in 1 7 42; do
  if ! out=$(timeout 120 "$BIN" "$seed" 3000 2>&1); then
    echo "KO fuzz graine $seed (crash ASan/UBSan) :"
    printf '%s\n' "$out" | tail -20
    rm -f "$BIN"; exit 1
  fi
  n=$(printf '%s\n' "$out" | grep -oE 'OK [0-9]+ checks' | grep -oE '[0-9]+' | tail -1)
  total=$((total + ${n:-0}))
done
rm -f "$BIN"
echo "OK $total checks (fuzz 3 graines × 3000 sous ASan/UBSan, 0 crash)"
