#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Acceptation du banc QNX : démarre l'invité, vérifie les preuves réelles attendues,
# arrête proprement et écrit un rapport horodaté. Une commande, aucun sudo.
#
# Usage : banc/verifie.sh [--laisse-en-marche] [--rapport FICHIER]
set -uo pipefail

DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
VM="${QNX_VM_DIR:-$HOME/qnx71-vm}"
RAPPORT="$VM/rapport-banc.txt"
LAISSER=0
ATTENTE_SSH="${QNX_ATTENTE_SSH:-300}"

while [ $# -gt 0 ]; do
  case "$1" in
    --laisse-en-marche) LAISSER=1; shift ;;
    --rapport) RAPPORT="${2:?}"; shift 2 ;;
    --help|-h) echo "usage: verifie.sh [--laisse-en-marche] [--rapport FICHIER]"; exit 0 ;;
    *) echo "KO option inconnue : $1" >&2; exit 2 ;;
  esac
done

checks=0; fail=0
ck() {  # ck <condition 0/1> <message>
  checks=$((checks + 1))
  if [ "$1" -eq 0 ]; then echo "OK   $2"; else echo "KO   $2"; fail=1; fi
}

out() { printf '%s\n' "$*" >> "$RAPPORT"; }
run_preset() {  # run_preset <nom> → met la sortie dans $SORTIE
  SORTIE=$(cd "$VM" && timeout 90 ./qnx-ssh "$2" 2>&1)
  out ""; out "### $1"; out "\$ $2"; out "$SORTIE"
}

echo "── acceptation du banc QNX ($VM) ──"
[ -d "$VM" ] || { echo "KO dossier du banc absent : $VM (QNX_VM_DIR)"; exit 3; }
[ -x "$VM/qnx-vm.sh" ] || { echo "KO harnais absent ou non exécutable"; exit 3; }

: > "$RAPPORT"
out "# Rapport d'acceptation du banc QNX — Tamis"
out "# Généré le $(date -u +%Y-%m-%dT%H:%M:%SZ) par banc/verifie.sh"
out "# Hôte : $(hostname) · utilisateur : $(id -un)"

# ── 1. prérequis ──────────────────────────────────────────────────────────
DOCTOR=$(cd "$VM" && ./qnx-vm.sh doctor 2>&1)
out ""; out "### doctor"; out "$DOCTOR"
command -v qemu-system-x86_64 >/dev/null 2>&1
ck $? "qemu-system-x86_64 installé"
printf '%s' "$DOCTOR" | grep -q "kvm        : accessible"
KVM_OK=$?
ck $KVM_OK "KVM accessible (sinon repli TCG, plus lent)"
[ -f "$VM/qnx71-fs.img" ]
ck $? "image disque présente"

# ── 2. démarrage ──────────────────────────────────────────────────────────
DEBUT=$(date +%s)
(cd "$VM" && ./qnx-vm.sh start) > "$VM/.start.log" 2>&1
out ""; out "### démarrage"; out "$(cat "$VM/.start.log")"
PRET=1
for _ in $(seq 1 $((ATTENTE_SSH / 10))); do
  if (cd "$VM" && timeout 25 ./qnx-ssh 'echo PRET' 2>/dev/null | grep -q PRET); then PRET=0; break; fi
  sleep 10
done
DUREE=$(( $(date +%s) - DEBUT ))
ck $PRET "SSH invité prêt (${DUREE} s)"
out "SSH prêt après ${DUREE} s"

if [ "$PRET" -ne 0 ]; then
  out ""; out "### ÉCHEC : SSH invité jamais prêt"
  out "$(tail -30 "$VM/qemu.out" 2>/dev/null)"
  echo "FAILED $checks vérifications, SSH invité inaccessible (rapport : $RAPPORT)"
  (cd "$VM" && ./qnx-vm.sh stop) >/dev/null 2>&1
  exit 1
fi

# ── 3. preuves réelles attendues ──────────────────────────────────────────
run_preset "uname"            'uname -a'
printf '%s' "$SORTIE" | grep -q "QNX"
ck $? "le noyau se déclare QNX"

run_preset "secpol"           'secpol 2>&1; echo rc=$?'
printf '%s' "$SORTIE" | grep -q "/proc/boot/secpol.bin"
ck $? "secpol lit bien /proc/boot/secpol.bin"

run_preset "secpolmonitor"    'secpolmonitor 2>&1; echo rc=$?'
printf '%s' "$SORTIE" | grep -qE "Nothing to do|-a, -p, -q or -u"
ck $? "secpolmonitor présent et bavard sur ses options"

run_preset "libsecpol"        'ls -la /proc/boot/libsecpol*'
printf '%s' "$SORTIE" | grep -q "libsecpol.so"
ck $? "libsecpol présente dans l'IFS"

run_preset "outils hotes"     'for b in secpolcompile secpolgenerate secpolpush; do command -v $b || echo "$b: absent"; done'
N=$(printf '%s\n' "$SORTIE" | grep -c "absent")
[ "$N" -eq 3 ]
ck $? "les 3 outils hôtes sont bien absents de la cible (preuve qu'ils sont dans le SDP)"

run_preset "procnto"          "pidin -F '%a %n' | head -5"
printf '%s' "$SORTIE" | grep -q "procnto"
ck $? "le noyau procnto tourne"

# ── 4. arrêt propre ───────────────────────────────────────────────────────
if [ "$LAISSER" -eq 1 ]; then
  out ""; out "### banc laissé en marche (--laisse-en-marche)"
  echo "INFO banc laissé en marche"
else
  (cd "$VM" && ./qnx-vm.sh stop) >> "$RAPPORT" 2>&1
  sleep 2
  pgrep -f "qnx71-fs.img" >/dev/null 2>&1
  [ $? -ne 0 ]
  ck $? "arrêt propre : plus aucun processus qemu sur cette image"
fi

out ""; out "### résultat"
out "$checks vérifications, $fail échec(s)"
echo
if [ "$fail" -ne 0 ]; then
  echo "FAILED $checks vérifications, $fail échec(s) — rapport : $RAPPORT"
  exit 1
fi
echo "OK $checks checks — rapport : $RAPPORT"
