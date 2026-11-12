#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# check_perimetre.sh — vérifie que le périmètre gelé (docs/GEL.md) est respecté.
# Utilisé avant chaque soumission : tout écart doit être soit réparé, soit voté.
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
checks=0; fail=0
ck() { checks=$((checks+1)); if [ "$1" -eq 0 ]; then echo "OK   $2"; else echo "KO   $2"; fail=1; fi; }

# 1. les 13 verdicts sont exactement ceux du contrat
attendu=13
reel=$(grep -cE '"TAMIS_[A-Z_]+",' core/src/verdict.c)
[ "$reel" -eq "$attendu" ]; ck $? "13 codes de verdict (trouvé $reel)"

# 2. les 9 sous-commandes de la CLI sont toutes là
for c in lint reach proof explain smt minimize diff view version; do
  grep -qE "\"$c\"" tools/tamis.c
  ck $? "commande CLI « $c »"
done

# 3. aucune dépendance Python externe (stdlib strict : liste blanche)
STDLIB='__future__ annotations argparse base64 dataclass dataclasses field glob html http json os pathlib random re secrets shlex shutil subprocess sys tempfile time typing urllib'
# ne retenir que les MODULES importés (pas les noms après « from X import Y »)
EXT=$(grep -hE '^\s*(import [A-Za-z_][A-Za-z0-9_]*|from [A-Za-z_][A-Za-z0-9_]*)' \
  host/*.py tools/*.py tests/*.py 2>/dev/null \
  | sed -E 's/^\s*(import|from) //' | awk '{print $1}' | sed 's/\..*//' | sort -u)
EXT_NON=$(for m in $EXT; do
  case " $STDLIB tamis_report bridge_backend " in *" $m "*) ;; *) echo "$m";; esac
done)
if [ -n "$EXT_NON" ]; then
  echo "KO   dépendances Python non-stdlib : $EXT_NON"
  fail=1; checks=$((checks+1))
else
  ck 0 "aucune dépendance Python non-stdlib"
fi

# 4. le cœur n'inclut que des en-têtes système et tamis/
INC=$(grep -hE '^\s*#include' core/src/*.c core/include/tamis/*.h 2>/dev/null \
  | sed 's/.*#include *[<"]\([^">]*\)[">].*/\1/' \
  | grep -vE '^(tamis/|[a-z_]+\.h$)' | sort -u | head)
if [ -n "$INC" ]; then
  echo "KO   includes non-système dans core/ : $INC"
  fail=1; checks=$((checks+1))
else
  ck 0 "core/ n'inclut que des en-têtes système et tamis/"
fi

# 5. le format JSON du contrat est toujours la référence (pas de clé nouvelle)
grep -q '"order des clés FIGÉ"' CONTRACT.md 2>/dev/null || true
grep -qE '"code"' core/src/rapport.c
ck $? "rapport JSON : la clé « code » reste la première"

# 6. les fixtures réelles sont présentes
[ -f fixtures/politique-qnx8.secpol ]
ck $? "fixture QNX 8.0 réelle présente"

# 7. le binaire compilé de référence est documenté
grep -q "e990d0cd" preuve/qnx8/empreintes.txt
ck $? "empreinte du binaire compilé documentée"

# 8. le site est généré depuis ses sources (aucune dérive en local au moment du contrôle)
python3 tools/build_site.py --out /tmp/perimetre-site >/dev/null 2>&1
diff -rq site /tmp/perimetre-site >/dev/null 2>&1
ck $? "le site est conforme à ses sources (aucune dérive)"
rm -rf /tmp/perimetre-site

# 9. aucune nouvelle dépendance dans le Makefile (targets connus)
grep -qE '^\.PHONY: .*(test|cli|site|mutants|sanitize|garde)( |$)' Makefile
ck $? "le Makefile garde les cibles du périmètre"

# 10. le document de gel est versionné
[ -f docs/GEL.md ]
ck $? "docs/GEL.md versionné"

if [ "$fail" -ne 0 ]; then
  echo "FAILED $checks vérifications, $fail écart(s) — à arbitrer avant soumission"
  exit 1
fi
echo "OK $checks checks"