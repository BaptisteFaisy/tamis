#!/bin/sh
# build-qnx.sh — construit l'image invitée QNX 8.0 de Tamis (x86_64).
#
#   image/build-qnx.sh [--sdp ~/qnx800] [--sortie ~/tamis-img]
#                       [--qnx-emu ~/Bureau/qnx-emu]
#
# Étapes :
#   1. cross-compilation du cœur C + de la CLI pour QNX 8.0 (qcc -V gcc_ntox86_64) ;
#   2. copie dans le répertoire de travail de la politique QNX 8.0 réelle à analyser ;
#   3. construction de l'IFS (mkifs) par le paquet réutilisable qnx-emu.
#
# La partie générique (chaîne qcc -> mkifs -> IFS, lancement QEMU, vitrine HTTP)
# vit dans qnx-emu : ce script ne la duplique pas, il l'appelle.
set -eu

RACINE="$(cd "$(dirname "$0")/.." && pwd)"
SDP="${QNX_SDP:-$HOME/qnx800}"
SORTIE="${SORTIE:-$HOME/tamis-img}"
QNXEMU="${QNXEMU:-$HOME/Bureau/qnx-emu}"

usage() {
    sed -n '2,13p' "$0" | sed 's/^# \{0,1\}//'
    exit "${1:-0}"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --sdp)     SDP="$2"; shift 2 ;;
        --sortie)  SORTIE="$2"; shift 2 ;;
        --qnx-emu) QNXEMU="$2"; shift 2 ;;
        --aide|-h) usage 0 ;;
        *) echo "build-qnx: argument inconnu : $1" >&2; exit 1 ;;
    esac
done

QNX_HOST="$SDP/host/linux/x86_64"
QNX_TARGET="$SDP/target/qnx"
export QNX_HOST QNX_TARGET
PATH="$QNX_HOST/usr/bin:$PATH"
export PATH

[ -x "$QNX_HOST/usr/bin/qcc" ] || { echo "build-qnx: qcc introuvable ($QNX_HOST)" >&2; exit 1; }
[ -f "$QNXEMU/build-image.sh" ] || { echo "build-qnx: qnx-emu introuvable ($QNXEMU)" >&2; exit 1; }

mkdir -p "$SORTIE"

# --- 1. cross-compilation du cœur + de la CLI -------------------------------
echo "== 1/3 cross-compilation QNX 8.0 (qcc -V gcc_ntox86_64) =="
SRC="$RACINE/core/src/*.c"
# shellcheck disable=SC2086
qcc -V gcc_ntox86_64 -std=c11 -Wall -Wextra -Werror \
    -I"$RACINE/core/include" -I"$RACINE/core/src" -I"$RACINE/tools" \
    -o "$SORTIE/tamis" "$RACINE/tools/tamis.c" "$RACINE/tools/sha256.c" $SRC
ls -l "$SORTIE/tamis"

# --- 2. fichiers de scénario embarqués --------------------------------------
echo "== 2/3 copie de la politique et du scénario =="
# rm -f avant cp : le fichier est en 0555 (copié en lecture seule), cp ne
# peut donc pas l'écraser en place.
rm -f "$SORTIE/politique.secpol" "$SORTIE/demo.sh"
cp "$RACINE/fixtures/politique-qnx8.secpol" "$SORTIE/politique.secpol"
cp "$RACINE/image/demo.sh" "$SORTIE/demo.sh"
chmod 0555 "$SORTIE/demo.sh"
ls -l "$SORTIE/politique.secpol" "$SORTIE/demo.sh"

# --- 3. IFS (mkifs) via le paquet réutilisable ------------------------------
echo "== 3/3 construction de l'IFS (qnx-emu) =="
"$QNXEMU/build-image.sh" --projet tamis \
    --buildfile "$RACINE/image/buildfile" \
    --sortie "$SORTIE" --sdp "$SDP" --sans-disque

echo ""
echo "OK :"
echo "   IFS → $SORTIE/ifs.bin"
echo "   boot : qnx-emu/guest-qemu.sh --ifs $SORTIE/ifs.bin --serie $HOME/tamis-demo/serial.log ..."
