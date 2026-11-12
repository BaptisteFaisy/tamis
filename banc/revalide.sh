#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# revalide.sh — revalider les attestations de QNX 7.1 sur une cible 8.0.
#
# Deux phases :
#   A. GRAMMAIRE  — si un SDP 8.0 est présent, on extrait sa grammaire et on compare
#                    le vocabulaire (mots-clés, modificateurs) aux attestations 7.1.
#   B. INVITÉ     — on rejoue les sondes du banc (secpol, secpolmonitor, libsecpol,
#                    outils hôtes, noyau) sur la cible et on compare aux valeurs de
#                    référence relevées sur QNX 7.1.
#
# Sorties : 0 = aucun écart ; 1 = écarts détectés (à arbitrer) ; 3 = prérequis manquant.
# Auto-validation : sans SDP et avec le banc 7.1, la phase B doit annoncer ZÉRO écart.
#
# Usage : banc/revalide.sh [--cible HÔTE] [--rapport FICHIER]
set -uo pipefail
DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
RACINE=$(cd "$DIR/.." && pwd)
CIBLE="${TAMIS_QNX_HOST:-}"
RAPPORT="$RACINE/preuve/qnx71/revalidation.txt"
BASE_ABIL="$RACINE/preuve/qnx71/abilities-binaires.txt"
BASE_SYNTAX="$RACINE/preuve/qnx71/syntaxe-attenstee.txt"
SDP="${QNX_SDP:-$HOME/qnx800}"
VM_DIR="${QNX_VM_DIR:-$HOME/qnx71-vm}"

while [ $# -gt 0 ]; do
  case "$1" in
    --cible) CIBLE="${2:?}"; shift 2 ;;
    --rapport) RAPPORT="${2:?}"; shift 2 ;;
    --help|-h) sed -n '2,18p' "$0"; exit 0 ;;
    *) echo "KO option inconnue : $1" >&2; exit 2 ;;
  esac
done

mkdir -p "$(dirname "$RAPPORT")"
: > "$RAPPORT"
ecarts=0; verifs=0

note() { echo "$*" | tee -a "$RAPPORT"; }
verif() { # verif <libellé> <obtenu> <attendu>
  verifs=$((verifs+1))
  if [ "$2" = "$3" ]; then
    note "OK   $1"
  else
    note "ÉCART $1 — obtenu [$2] attendu [$3]"
    ecarts=$((ecarts+1))
  fi
}

note "== Revalidation QNX 7.1 -> cible = ${CIBLE:-local} — $(date -u +%Y-%m-%dT%H:%M:%SZ) =="

# ── Phase A : grammaire (nécessite le SDP 8.0) ──────────────────────────────
note ""
note "-- Phase A : grammaire (SDP 8.0) --"
if [ ! -d "$SDP" ]; then
  note "INFO pas de SDP à $SDP : phase A ignorée (prérequis manquant)"
else
  DOC=$(find "$SDP" -name 'secpol_language.html' 2>/dev/null | head -1)
  if [ -z "$DOC" ]; then
    note "KO  secpol_language.html introuvable sous $SDP"
    ecarts=$((ecarts+1))
  else
    TMP=$(mktemp -d)
    python3 "$RACINE/tools/extract_grammar.py" "$(dirname "$DOC")" --json > "$TMP/g.json" 2>/dev/null
    python3 - "$TMP/g.json" "$BASE_SYNTAX" <<'PY' | tee -a "$RAPPORT"
import json, re, sys
data = json.load(open(sys.argv[1], encoding="utf-8"))
mots = set()
for page in data.get("pages", []):
    mots |= set(page.get("mots_cles", []))
    mots |= set(page.get("modificateurs", []))
base = set()
for ligne in open(sys.argv[2], encoding="utf-8"):
    for mot in re.findall(r"[A-Za-z_][A-Za-z_0-9-]*", ligne):
        base.add(mot)
manquants = sorted(m for m in base if m not in mots and m.islower() and len(m) > 2)
print(f"OK   vocabulaire extrait : {len(mots)} termes")
if manquants:
    print(f"ÉCART termes attestés en 7.1 absents de la doc 8.0 : {', '.join(manquants)}")
else:
    print("OK   aucun terme attesté en 7.1 ne manque dans la doc 8.0")
PY
    rm -rf "$TMP"
  fi
fi

# ── Phase B : sondes sur l'invité ────────────────────────────────────────────
note ""
note "-- Phase B : sondes sur l'invité --"
if [ -n "$CIBLE" ]; then
  Q="ssh -o BatchMode=yes -o ConnectTimeout=10 $CIBLE"
else
  Q=""
fi

sonde() { # sonde <commande invité>
  if [ -n "$CIBLE" ]; then
    ssh -o BatchMode=yes -o ConnectTimeout=10 "$CIBLE" \
      "cd $VM_DIR && ./qnx-ssh \"$1\"" 2>/dev/null
  else
    (cd "$VM_DIR" 2>/dev/null && ./qnx-ssh "$1" 2>/dev/null) || true
  fi
}

if ! sonde "echo PRET" | grep -q PRET; then
  note "INFO aucun invité joignable : phase B ignorée (prérequis manquant)"
  note ""
  note "== $verifs vérifications, $ecarts écart(s) — prérequis manquants, non concluant =="
  exit 3
fi

# noyau : QNX 7.1 sur le banc de référence, 8.0 attendu sinon
KERNEL=$(sonde "uname -s" | tr -d '\r')
verif "le noyau est bien QNX" "$KERNEL" "QNX"

# secpol sans politique : message réel
S1=$(sonde "secpol 2>&1" | tr -d '\r' | head -1)
verif "secpol sans politique" "$S1" "/proc/boot/secpol.bin: No such file or directory"

# secpolmonitor : message d'usage réel
S2=$(sonde "secpolmonitor 2>&1" | tr -d '\r' | head -1)
verif "secpolmonitor sans option" "$S2" "Nothing to do, none of the options -a, -p, -q or -u were specified"

# libsecpol dans l'IFS
S3=$(sonde "ls /proc/boot/libsecpol.so.1 2>/dev/null | wc -l" | tr -d '\r ' | head -1)
verif "libsecpol.so.1 présente dans /proc/boot" "${S3:-0}" "1"

# les outils hôtes sont absents de la cible
S4=$(sonde "command -v secpolcompile || echo absent" | tr -d '\r' | head -1)
verif "secpolcompile absent de la cible" "${S4:-absent}" "absent"

# le compteur de visites du rapport : la preuve reste exposable
note ""
note "== $verifs vérifications, $ecarts écart(s) =="
if [ "$ecarts" -ne 0 ]; then
  note "ÉCARTS DÉTECTÉS — à arbitrer avant de figer la documentation QNX 8.0"
  exit 1
fi
note "AUCUN ÉCART — les attestations 7.1 tiennent sur cette cible"
exit 0
