#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# check_soumission.sh — checklist « prêt à soumettre » : 20 points binaires.
# Seuls le tournage de la vidéo (19) et la soumission Devpost (20) restent humains ;
# tout le reste est vérifiable par cette commande. Usage : bash tools/check_soumission.sh
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
SITE="https://tradoooo.github.io/tamis"
checks=0; fail=0; warn=0
ok()   { checks=$((checks+1)); echo "✅  $1"; }
ko()   { checks=$((checks+1)); echo "❌  $1"; fail=$((fail+1)); }
warn() { checks=$((checks+1)); echo "⚠️  $1"; warn=$((warn+1)); }

# 1. licence MIT
grep -qi "MIT License" LICENSE 2>/dev/null && ok "1. licence MIT visible" || ko "1. licence MIT"

# 2. README + mentions
grep -q "github.com/Tradoooo/tamis" README.md 2>/dev/null && ok "2. README mentionne le dépôt" || ko "2. README"

# 3. site déployé (suivre la redirection du slash final)
code=$(curl -sL -o /dev/null -w '%{http_code}' --max-time 15 "$SITE/" 2>/dev/null)
[ "$code" = "200" ] && ok "3. site en ligne (200)" || ko "3. site en ligne (obtenu ${code:-aucun})"

# 4. les 7 onglets attendus
H=$(curl -sL --max-time 15 "$SITE/" 2>/dev/null)
manquants=""
for t in playground features proofs submission codelab ideas grammar; do
  grep -q "id=\"$t\"" <<< "$H" || manquants="$manquants $t"
done
[ -z "$manquants" ] && ok "4. les 7 onglets du site présents" || ko "4. onglet(s) absent(s) :$manquants"

# 5. dossier de soumission Devpost : 13 sections
n=$(grep -cE '^## [0-9]+\. ' docs/devpost-form.md 2>/dev/null)
[ "${n:-0}" -ge 13 ] && ok "5. formulaire Devpost (13 sections)" || ko "5. formulaire Devpost (${n:-0}/13)"

# 6. codelab
[ -s docs/codelab/01-auditer-une-politique.md ] && ok "6. codelab présent" || ko "6. codelab"

# 7. tableau de traçabilité des affirmations
[ -s docs/AFFIRMATIONS-QNX.md ] && ok "7. traçabilité des affirmations QNX" || ko "7. traçabilité des affirmations (docs/AFFIRMATIONS-QNX.md)"

# 8. rétroplanning
[ -s docs/RETROPLANNING.md ] && ok "8. rétroplanning daté" || ko "8. rétroplanning"

# 9. gel de périmètre + vérification
[ -s docs/GEL.md ] && bash tools/check_perimetre.sh >/dev/null 2>&1 \
  && ok "9. gel de périmètre respecté" || ko "9. gel de périmètre (docs/GEL.md / check_perimetre.sh)"

# 10. fixtures réelles
[ -s fixtures/politique-qnx8.secpol ] && ok "10. fixture QNX 8.0 réelle" || ko "10. fixture réelle"

# 11. preuves QNX (7.1 + 8.0)
[ -s preuve/qnx71/LISEZMOI.md ] && [ -s preuve/qnx8/LISEZMOI.md ] \
  && ok "11. preuves QNX 7.1 et 8.0 publiées" || ko "11. preuves QNX"

# 12. tests verts
timeout 500 make test >/dev/null 2>&1 && ok "12. make test vert" || ko "12. make test"

# 13. garde de taille
python3 tests/garde.py >/dev/null 2>&1 && ok "13. garde de taille verte" || ko "13. garde de taille"

# 14. mutants tués
python3 tests/mutants.py >/dev/null 2>&1 && ok "14. mutants tués" || ko "14. mutants"

# 15. aucun chemin absolu codé en dur (le motif est assemblé pour ne pas se matcher soi-même)
MOTIF="/hom"'e/'
HITS=$(grep -rn --include='*.sh' --include='*.py' --include='*.c' --include='*.h' "$MOTIF" . \
  | grep -v '/fixtures/' | grep -v '/tests/')
[ -z "$HITS" ] && ok "15. aucun chemin absolu codé en dur" || ko "15. chemin absolu codé en dur"

# 16. budget CI ≤ 5 min
grep -q 'timeout-minutes: 5' .github/workflows/ci.yml && ok "16. budget CI ≤ 5 min" || ko "16. budget CI"

# 17. le site est généré depuis ses sources
python3 tools/build_site.py --out /tmp/site-soumission >/dev/null 2>&1
diff -rq site /tmp/site-soumission >/dev/null 2>&1 && ok "17. site conforme à ses sources" || ko "17. site non conforme (relancer build_site.py)"
rm -rf /tmp/site-soumission

# 18. aucun TODO ouvert dans les documents publics
grep -rqiE '\b(TODO|FIXME|XXX)\b' README.md CONTRACT.md docs/*.md docs/codelab docs/idees 2>/dev/null \
  && ko "18. TODO/FIXME dans les documents publics" || ok "18. aucun TODO/FIXME public"

# 19. vidéo (humain)
VID=$(ls files/*.mp4 files/*.mov files/*.webm video.* 2>/dev/null)
[ -n "$VID" ] && ok "19. vidéo présente" || warn "19. vidéo 3 min À FILMER (humain) + la version 90 s"

# 20. soumission Devpost (humain)
warn "20. soumission Devpost (compte de l'équipe) — dossier prêt dans docs/devpost-form.md"

echo
if [ "$fail" -ne 0 ]; then
  echo "FAILED $checks points, $fail échec(s), $warn avertissement(s)"
  exit 1
fi
echo "OK $checks points, $warn avertissement(s) humain(s) restant(s)"