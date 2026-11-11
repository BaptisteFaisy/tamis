#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Runne les tests hôtes : host/test_*.py (Python) et tests/test_*.sh (différentiel, fuzz…).
set -uo pipefail
total=0; echecs=0; ran=0
run_one() {
  local t="$1" sortie code n
  ran=$((ran+1))
  case "$t" in
    *.py) if sortie=$(timeout 180 python3 "$t" 2>&1); then code=0; else code=$?; fi ;;
    *)    if sortie=$(timeout 180 bash "$t" 2>&1); then code=0; else code=$?; fi ;;
  esac
  if [ "$code" -eq 0 ]; then
    n=$(printf '%s\n' "$sortie" | grep -oE 'OK [0-9]+ checks' | grep -oE '[0-9]+' | tail -1)
    n=${n:-0}
    printf 'OK   %s (%s checks)\n' "$t" "$n"
    total=$((total+n))
  else
    printf 'KO   %s (code %s)\n' "$t" "$code"
    printf '%s\n' "$sortie" | sed 's/^/     /' | tail -15
    echecs=$((echecs+1))
  fi
}
shopt -s nullglob
for t in host/test_*.py tests/test_*.sh; do run_one "$t"; done
[ "$ran" -eq 0 ] && echo "ATTENTION aucun test hôte (host/test_*.py, tests/test_*.sh)"
if [ "$echecs" -ne 0 ]; then
  echo "FAILED $echecs test(s) hôte en échec"
  exit 1
fi
echo "OK $total checks ($ran tests hôte)"
