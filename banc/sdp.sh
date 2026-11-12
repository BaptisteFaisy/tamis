#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# sdp.sh — détecte un SDP QNX installé et active tout ce qu'il rend possible.
#
# C'est le point de bascule de la licence gratuite : une fois le SDP 8.0 installé, cette
# commande (a) vérifie les outils hôtes, (b) localise la documentation secpol installée,
# (c) extrait automatiquement la grammaire (étape E1 du cahier), (d) relance le différentiel
# oracle contre secpolcompile (TAM-12).
#
# Usage : banc/sdp.sh [--chemin SDP] [--extraire-seulement]
set -uo pipefail
DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
RACINE=$(cd "$DIR/.." && pwd)
SDP="${QNX_SDP:-$HOME/qnx800}"
EXTRAIRE=0

while [ $# -gt 0 ]; do
  case "$1" in
    --chemin) SDP="${2:?}"; shift 2 ;;
    --extraire-seulement) EXTRAIRE=1; shift ;;
    --help|-h) sed -n '2,18p' "$0"; exit 0 ;;
    *) echo "KO option inconnue : $1" >&2; exit 2 ;;
  esac
done

checks=0; fail=0
ck() { checks=$((checks+1)); if [ "$1" -eq 0 ]; then echo "OK   $2"; else echo "KO   $2"; fail=1; fi; }

echo "── SDP QNX ────────────────────────────────────────"
echo "chemin : $SDP"

if [ ! -d "$SDP" ]; then
  echo "KO aucun SDP installé à $SDP"
  cat <<'MSG'

Pour l'obtenir (licence gratuite, ~10 min, action humaine) :
  1. créer un compte myQNX  : https://www.qnx.com/developers/download/
  2. télécharger « QNX SDP 8.0 » (hôte Linux) depuis la même page
  3. l'installer, par exemple dans ~/qnx800
  4. relancer :  QNX_SDP=~/qnx800 bash banc/sdp.sh

Sans le SDP, le projet reste pleinement fonctionnel : le parseur du langage officiel
attend la grammaire (docs/GRAMMAR.md), et le différentiel oracle sort 3 (prérequis
manquant) — c'est le seul maillon qui manque pour la boucle « prédit ↔ observé ».
MSG
  exit 3
fi
ck 0 "SDP présent : $SDP"

echo
echo "── outils hôtes (ce qui manque à l'invité, et qui permet le différentiel) ──"
for outil in secpolcompile secpolgenerate secpolpush; do
  trouve=$(find "$SDP" -type f -name "$outil" 2>/dev/null | head -1)
  if [ -n "$trouve" ]; then
    ck 0 "$outil : $trouve"
  else
    ck 1 "$outil ABSENT du SDP"
  fi
done

echo
echo "── documentation secpol installée ──"
DOC=$(find "$SDP" -name 'secpol_language.html' 2>/dev/null | head -1)
if [ -n "$DOC" ]; then
  ck 0 "page de référence : $DOC"
  DOSSIER=$(dirname "$DOC")
  echo "     extraction de la grammaire (étape E1) …"
  if python3 "$RACINE/tools/extract_grammar.py" "$DOSSIER" \
        --out "$RACINE/docs/GRAMMAR-auto.md" >/dev/null 2>&1; then
    n=$(grep -c '^| `' "$RACINE/docs/GRAMMAR-auto.md" 2>/dev/null || echo 0)
    ck 0 "brouillon de grammaire extrait : docs/GRAMMAR-auto.md ($n productions) — à relire puis fusionner dans docs/GRAMMAR.md"
  else
    ck 1 "extraction de la grammaire échouée"
  fi
else
  ck 1 "secpol_language.html introuvable sous $SDP"
fi

if [ "$EXTRAIRE" -eq 1 ]; then
  echo
  echo "OK $checks vérifications (extraction seulement)"
  exit 0
fi

echo
echo "── différentiel oracle contre secpolcompile (TAM-12) ──"
if "$RACINE/oracle/run_oracle.sh" 2>&1 | sed 's/^/    /'; then
  ck 0 "différentiel oracle exécuté"
else
  ck 1 "différentiel oracle en échec (voir ci-dessus)"
fi

echo
if [ "$fail" -ne 0 ]; then
  echo "FAILED $checks vérifications, $fail échec(s)"
  exit 1
fi
echo "OK $checks checks — le SDP est pleinement exploité"
