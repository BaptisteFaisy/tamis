#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lit le tampon texte VGA (0xB8000) de l'invité QNX via le moniteur QEMU.

L'invité n'a pas de console série : le seul canal texte est le framebuffer VGA.
Le moniteur `xp /Nc 0xb8000` rend des paires (caractère, attribut) entre apostrophes.
Usage : vga-dump.py [chemin-du-socket-moniteur] [nombre-de-caractères]
"""
from __future__ import annotations

import re
import socket
import sys
import time
from pathlib import Path

SOCK = sys.argv[1] if len(sys.argv) > 1 else str(Path.home() / "qnx71-vm" / "mon.sock")
N = sys.argv[2] if len(sys.argv) > 2 else "2000"

# Un jeton = 'x' ou '\xHH' ou '\n' etc., entre apostrophes.
JETON = re.compile(r"'((?:\\x[0-9a-f]{2}|\\.|[^'\\]))'")


def interroger(sock: socket.socket, cmd: str, attente: float = 4.0) -> str:
    sock.sendall((cmd + "\n").encode())
    time.sleep(attente)
    out = b""
    try:
        while True:
            bloc = sock.recv(65536)
            if not bloc:
                break
            out += bloc
    except socket.timeout:
        pass
    return out.decode("latin-1")


def decode(t: str) -> str:
    if t.startswith("\\x"):
        return chr(int(t[2:], 16))
    if t == "\\n":
        return "\n"
    if t == "\\t":
        return "\t"
    if t.startswith("\\"):
        return t[1]
    return t


def main() -> int:
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.settimeout(10)
    try:
        s.connect(SOCK)
    except OSError as e:
        print(f"KO moniteur injoignable ({SOCK}) : {e}")
        return 3
    raw = interroger(s, f"xp /{N}c 0xb8000")
    if ":" not in raw:
        print("KO réponse du moniteur illisible")
        print(raw[:300])
        return 1
    corps = raw.split(":", 1)[1]
    jetons = [decode(t) for t in JETON.findall(corps)]
    # un jeton sur deux est l'attribut de couleur : on ne garde que les caractères
    caracteres = [j for j in jetons[0::2] if len(j) == 1]
    texte = "".join(c if 32 <= ord(c) < 127 else ("\n" if c == "\x00" else " ")
                    for c in caracteres)
    lignes = [texte[i:i + 80] for i in range(0, len(texte), 80)]
    utiles = [l.rstrip() for l in lignes]
    print("=== CONSOLE VGA de l'invité QNX ===")
    for l in utiles[:30]:
        if l:
            print("|" + l)
    s.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
