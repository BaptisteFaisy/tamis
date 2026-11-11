#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Runne chaque binaire de test C d'un dossier (défaut core/tests), attend "OK <n> checks".
set -uo pipefail
DIR="${1:-core/tests}"
total=0; echecs=0; ran=0
shopt -s nullglob
for bin in "$DIR"/test_*; do
  case "$bin" in *.c|*.py|*.o|*.h) continue ;; esac
  [ -x "$bin" ] || continue
  ran=$((ran+1))
  if sortie=$(timeout 120 "$bin" 2>&1); then
    n=$(printf '%s\n' "$sortie" | grep -oE 'OK [0-9]+ checks' | grep -oE '[0-9]+' | tail -1)
    n=${n:-0}
    printf 'OK   %s (%s checks)\n' "$bin" "$n"
    total=$((total+n))
  else
    code=$?
    printf 'KO   %s (code %s)\n' "$bin" "$code"
    printf '%s\n' "$sortie" | sed 's/^/     /' | tail -12
    echecs=$((echecs+1))
  fi
done
[ "$ran" -eq 0 ] && echo "ATTENTION aucun binaire de test dans $DIR"
if [ "$echecs" -ne 0 ]; then
  echo "FAILED $echecs binaire(s) de test en échec"
  exit 1
fi
echo "OK $total checks ($ran binaires C)"
