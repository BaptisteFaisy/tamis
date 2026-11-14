# SPDX-License-Identifier: MIT
# Compteur de vérifications partagé par les scripts du banc (à SOURCER, pas à exécuter).
# Convention : ck <code> <message> avec code 0 = OK, non nul = KO (comme `grep $?`).
#
# Usage :
#   source "$(dirname "$0")/ck.sh"
#   ck 0 "l'unité répond"
#   ck_final            # → « OK N vérifications » et rc 0, ou « FAILED N… » et rc 1

checks=0
fail=0

ck() {  # ck <code 0|non nul> <message>
  checks=$((checks + 1))
  if [ "$1" -eq 0 ]; then echo "OK   $2"; else echo "KO   $2"; fail=1; fi
}

ck_final() {  # ck_final [<suffixe>]
  echo
  if [ "$fail" -ne 0 ]; then
    echo "FAILED $checks vérifications, $fail échec(s)${1:+ — $1}"
    return 1
  fi
  echo "OK $checks vérifications${1:+ — $1}"
  return 0
}
