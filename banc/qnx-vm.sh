#!/bin/sh
# SPDX-License-Identifier: MIT
#
# qnx-vm.sh — harnais du banc QNX (invité QNX Neutrino 7.1 sous QEMU).
# Portable : même script sur le poste de dev et sur le serveur.
#
#   ./qnx-vm.sh start      démarre en arrière-plan (KVM si disponible, sinon TCG)
#   ./qnx-vm.sh foreground démarre au premier plan (pour systemd)
#   ./qnx-vm.sh stop       arrête
#   ./qnx-vm.sh status     état + accélérateur
#   ./qnx-vm.sh ssh        shell interactif dans l'invité
#   ./qnx-vm.sh doctor     diagnostic complet (prérequis, image, réseau)
#
# Variables : QNX_IMG, QNX_RAM (1024), QNX_SMP (2), QNX_SSH_PORT (2225),
#             QNX_GUEST_IP (192.168.100.10), QNX_NET (192.168.100.0/24), QNX_FORCE_ACCEL
set -eu

DIR=$(cd "$(dirname "$0")" && pwd)
IMG="${QNX_IMG:-$DIR/qnx71-fs.img}"
RAM="${QNX_RAM:-1024}"
SMP="${QNX_SMP:-2}"
PORT="${QNX_SSH_PORT:-2225}"
GUEST_IP="${QNX_GUEST_IP:-192.168.100.10}"
NET="${QNX_NET:-192.168.100.0/24}"
# Préfixe utilisateur : qemu extrait sans sudo (voir banc/LISEZMOI.md).
QPREFIX="${QNX_QEMU_PREFIX:-$DIR/qprefix}"
# Machine épinglée : cette image QNX 7.1 monte ses partitions QNX6 via devb-eide (IDE).
# Sous qemu >= 9 le type par défaut peut différer et le système racine ne se monte plus.
MACHINE="${QNX_MACHINE:-pc-i440fx-8.2}"
TCG_THREAD="${QNX_TCG_THREAD:-multi}"
MON="$DIR/mon.sock"
PIDFILE="$DIR/qemu.pid"
LOG="$DIR/qemu.out"
SERIAL="$DIR/serial.log"

# KVM si le périphérique est accessible en lecture/écriture, sinon TCG multithread.
accel() {
    if [ -n "${QNX_FORCE_ACCEL:-}" ]; then
        echo "$QNX_FORCE_ACCEL"; return
    fi
    if [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
        echo kvm
    else
        echo tcg
    fi
}

# binaire qemu : système d'abord, puis préfixe utilisateur.
qemu_bin() {
    for c in qemu-system-x86_64 /usr/bin/qemu-system-x86_64; do
        command -v "$c" >/dev/null 2>&1 && { echo "$c"; return; }
    done
    [ -x "$QPREFIX/usr/bin/qemu-system-x86_64" ] && { echo "$QPREFIX/usr/bin/qemu-system-x86_64"; return; }
    return 1
}

# Prépare l'environnement (LD_LIBRARY_PATH) et le firmware (-L) selon le binaire choisi.
qemu_env() {
    QEMU="$1"
    QEMU_L=""
    case "$QEMU" in
        "$QPREFIX"/*)
            LD_LIBRARY_PATH="$QPREFIX/usr/lib/x86_64-linux-gnu:$QPREFIX/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
            export LD_LIBRARY_PATH
            if [ -d "$QPREFIX/usr/share/qemu" ]; then
                QEMU_L="-L $QPREFIX/usr/share/qemu"
            fi
            ;;
    esac
}

netdev() {
    echo "user,id=n0,net=$NET,dhcpstart=${NET%.*}.60,hostfwd=tcp:127.0.0.1:$PORT-$GUEST_IP:22"
}

running() {
    [ -f "$PIDFILE" ] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null
}

start() {
    [ -f "$IMG" ] || { echo "KO image absente : $IMG"; exit 3; }
    QEMU=$(qemu_bin) || { echo "KO qemu-system-x86_64 absent (sudo apt-get install qemu-system-x86)"; exit 3; }
    qemu_env "$QEMU"
    # UNE seule façon de faire tourner l'invité : le service systemd utilisateur.
    # (Deux modes de lancement se disputent le port de redirection SSH.)
    if [ -f "${XDG_CONFIG_HOME:-$HOME/.config}/systemd/user/tamis-qnx.service" ] &&
       command -v systemctl >/dev/null 2>&1; then
        systemctl --user start tamis-qnx.service >/dev/null 2>&1 ||
            { echo "KO le service tamis-qnx n'a pas démarré (voir : journalctl --user -u tamis-qnx)"; exit 1; }
        echo "OK QNX démarré via le service systemd utilisateur"
        return 0
    fi
    if running; then echo "déjà en marche (pid $(cat "$PIDFILE"))"; return 0; fi
    A=$(accel)
    rm -f "$MON" "$SERIAL"
    if [ "$A" = "tcg" ]; then
        echo "ATTENTION /dev/kvm inaccessible → émulation TCG (démarrage lent, plusieurs minutes)"
        ACC="-accel tcg,thread=$TCG_THREAD"
    else
        ACC="-accel kvm"
    fi
    # shellcheck disable=SC2086
    setsid nohup "$QEMU" $ACC $QEMU_L -machine "$MACHINE" \
        -drive format=raw,file="$IMG" \
        -m "$RAM" -smp "$SMP" \
        -netdev "$(netdev)" -device vmxnet3,netdev=n0 \
        -device usb-ehci -device usb-kbd -usb \
        -display none -serial "file:$SERIAL" \
        -monitor "unix:$MON,server,nowait" \
        > "$LOG" 2>&1 < /dev/null &
    echo $! > "$PIDFILE"
    echo "OK QNX démarré (pid $(cat "$PIDFILE"), accélérateur $A, ssh 127.0.0.1:$PORT)"
}

stop() {
    if running; then
        kill "$(cat "$PIDFILE")" 2>/dev/null || true
        rm -f "$PIDFILE"
        echo "OK arrêté"
    else
        rm -f "$PIDFILE"
        echo "déjà arrêté"
    fi
    # si l'invité tourne comme service systemd utilisateur, on l'arrête aussi —
    # sauf quand on EST le service (ExecStop) : s'arrêter soi-même annulerait le job.
    if [ -z "${INVOCATION_ID:-}" ] && command -v systemctl >/dev/null 2>&1; then
        systemctl --user stop tamis-qnx.service >/dev/null 2>&1 && echo "OK service tamis-qnx.service arrêté"
    fi
}

status() {
    if running; then
        echo "en marche (pid $(cat "$PIDFILE"))"
    else
        echo "arrete"
    fi
    echo "accélérateur : $(accel)"
}

doctor() {
    echo "── banc QNX ──────────────────────────────"
    printf 'image      : %s\n' "$IMG"
    if [ -f "$IMG" ]; then
        printf '             %s octets\n' "$(stat -c %s "$IMG" 2>/dev/null || echo '?')"
    else
        echo "             ABSENTE"
    fi
    Q=$(qemu_bin 2>/dev/null) || Q=""
    if [ -n "$Q" ]; then
        case "$Q" in
            "$QPREFIX"/*) printf 'qemu       : %s  (préfixe utilisateur, sans sudo)\n' "$Q" ;;
            *)              printf 'qemu       : %s  (système)\n' "$Q" ;;
        esac
    else
        echo "qemu       : ABSENT (sudo apt-get install qemu-system-x86, ou banc/LISEZMOI.md §préfixe)"
    fi
    if [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
        echo "kvm        : accessible"
    else
        echo "kvm        : INACCESSIBLE (sudo usermod -aG kvm \$USER, puis se reconnecter)"
    fi
    echo "accélération: $(accel)"
    echo "ssh invité : 127.0.0.1:$PORT -> $GUEST_IP:22"
    if running; then
        echo "état       : en marche (pid $(cat "$PIDFILE"))"
        printf 'ssh répond : '; ./qnx-ssh 'echo PRET' 2>/dev/null | grep -q PRET && echo oui || echo "non (pas encore)"
    else
        echo "état       : arrêté"
    fi
}

case "${1:-status}" in
    start)      start ;;
    foreground) [ -f "$IMG" ] || { echo "KO image absente"; exit 3; }
                QEMU=$(qemu_bin) || { echo "KO qemu absent"; exit 3; }
                qemu_env "$QEMU"
                A=$(accel); rm -f "$MON"
                if [ "$A" = "tcg" ]; then ACC="-accel tcg,thread=$TCG_THREAD"; else ACC="-accel kvm"; fi
                # le PID est conservé (exec remplace le shell) : status/doctor le voient
                echo $$ > "$PIDFILE"
                # shellcheck disable=SC2086
                exec "$QEMU" $ACC $QEMU_L -machine "$MACHINE" -drive format=raw,file="$IMG" -m "$RAM" -smp "$SMP" \
                    -netdev "$(netdev)" -device vmxnet3,netdev=n0 \
                    -device usb-ehci -device usb-kbd -usb \
                    -display none -serial "file:$SERIAL" -monitor "unix:$MON,server,nowait" ;;
    stop)       stop ;;
    status)     status ;;
    doctor)     doctor ;;
    ssh)        shift; exec "$DIR/qnx-ssh" "$@" ;;
    *)          echo "usage: $0 {start|foreground|stop|status|doctor|ssh}"; exit 2 ;;
esac
