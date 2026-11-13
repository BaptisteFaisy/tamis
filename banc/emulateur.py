#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pilote de l'invité QNX 8.0 (émulateur QEMU) : exécuter une commande, capturer sa sortie.

L'invité QNX 8.0 de Tamis est une vraie machine QEMU (service `tamis-qemu8`), lancée avec
`--monitor unix:…`, `--vga std` et `--serial file:…`. On entre par le clavier de la console
VGA (moniteur QEMU : `sendkey`) et on **récupère la sortie par la console série** : la
commande est exécutée avec `> /dev/ser1 2>&1`, donc le journal série contient exactement
les octets produits par la commande — accents intacts, contrairement au tampon VGA.

Aucune dépendance hors bibliothèque standard.

Usage :
    python3 banc/emulateur.py <commande>...      # exécute et affiche la sortie
    python3 banc/emulateur.py --identite          # identité de l'invité (uname, pidin)
"""
from __future__ import annotations

import os
import pathlib
import re
import socket
import sys
import time

MONITOR = os.environ.get("TAMIS_QNX_MONITOR", str(pathlib.Path.home() / "tamis-demo" / "mon.sock"))
SERIE = os.environ.get("TAMIS_QNX_SERIE", str(pathlib.Path.home() / "tamis-demo" / "serial.log"))
DEBUT = "__TAMIS_DEBUT__"
FIN = "__TAMIS_FIN__"
ATTENTE = float(os.environ.get("TAMIS_QNX_ATTENTE", "20"))

# Noms de touches QEMU (sendkey) pour les caractères non alphanumériques.
TOUCHES = {
    " ": "spc", "-": "minus", ".": "dot", "/": "slash", "=": "equal",
    ";": "shift-semicolon", ":": "shift-semicolon", "$": "shift-4", "(": "shift-9",
    ")": "shift-0", "_": "shift-minus", "'": "apostrophe", ",": "comma", "|": "shift-backslash",
    ">": "shift-dot", "<": "shift-comma", "&": "shift-7", "+": "shift-equal", "*": "shift-8",
    "#": "shift-3", "[": "bracket_left", "]": "bracket_right", "{": "shift-bracket_left",
    "}": "shift-bracket_right", "?": "shift-slash", "!": "shift-1", "%": "shift-5",
    "@": "shift-2", "^": "shift-6", "~": "shift-grave", "`": "grave",
}


class Erreur(Exception):
    """Échec de pilotage de l'invité."""


class Moniteur:
    """Client du moniteur QEMU (socket UNIX, une seule connexion à la fois)."""

    def __init__(self, chemin: str = MONITOR) -> None:
        self.chemin = chemin
        self.sock: socket.socket | None = None

    def connecter(self, essais: int = 15) -> None:
        if self.sock is not None:
            return
        dernier = None
        for _ in range(essais):
            try:
                s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                s.settimeout(10)
                s.connect(self.chemin)
                self.sock = s
                time.sleep(0.3)
                self._vider()
                return
            except OSError as e:          # EAGAIN : le moniteur est déjà occupé
                dernier = e
                time.sleep(1.0)
        raise Erreur(f"moniteur QEMU inaccessible ({self.chemin}) : {dernier}")

    def _vider(self) -> None:
        assert self.sock is not None
        try:
            self.sock.settimeout(0.4)
            self.sock.recv(65536)
        except OSError:
            pass
        self.sock.settimeout(10)

    def _commande(self, texte: str, delai: float = 0.25, drain: float = 0.06) -> str:
        assert self.sock is not None
        self.sock.sendall((texte + "\n").encode())
        time.sleep(delai)
        sortie = b""
        self.sock.settimeout(drain)
        try:
            while True:
                morceau = self.sock.recv(65536)
                if not morceau:
                    break
                sortie += morceau
        except OSError:
            pass
        self.sock.settimeout(10)
        return sortie.decode("latin-1")

    def fermer(self) -> None:
        if self.sock is not None:
            try:
                self.sock.close()
            except OSError:
                pass
            self.sock = None


class Invite:
    """L'invité QNX : on tape sur la console VGA, on lit le journal série."""

    def __init__(self, moniteur: str = MONITOR, serie: str = SERIE) -> None:
        self.mon = Moniteur(moniteur)
        self.serie = pathlib.Path(serie)

    def ouvrir(self) -> None:
        self.mon.connecter()

    def fermer(self) -> None:
        self.mon.fermer()

    def taper(self, texte: str, pause: float = 0.05) -> None:
        """Tape un texte caractère par caractère dans la console VGA de l'invité."""
        for ch in texte:
            if ch == "\n":
                touche = "ret"
            elif ch.isupper():
                touche = "shift-" + ch.lower()
            else:
                touche = TOUCHES.get(ch, ch)
            self.mon._commande(f"sendkey {touche}", delai=pause, drain=0.05)

    def _serie_d(self, position: int) -> str:
        """Journal série depuis `position`. Décodé en UTF-8 : c'est l'encodage du terminal QNX."""
        try:
            with self.serie.open("rb") as f:
                f.seek(position)
                return f.read().decode("utf-8", errors="replace")
        except OSError:
            return ""

    def executer(self, commande: str, attente: float = ATTENTE) -> str:
        """Exécute `commande` dans l'invité et renvoie sa sortie exacte (texte)."""
        position = self.serie.stat().st_size if self.serie.exists() else 0
        self.taper(f"echo {DEBUT} > /dev/ser1\n{commande} > /dev/ser1 2>&1\necho {FIN} > /dev/ser1\n")
        limite = time.time() + attente
        delta = ""
        while time.time() < limite:
            time.sleep(0.5)
            delta = self._serie_d(position)
            if delta.count(FIN) >= 1 and delta.count(DEBUT) >= 1:
                break
        if FIN not in delta or DEBUT not in delta:
            raise Erreur(f"pas de fin de commande dans l'invité pour : {commande}")
        corps = delta.split(DEBUT, 1)[1].split(FIN, 1)[0]
        # La console série termine ses lignes en CRLF ; on normalise en LF pour comparer
        # octet pour octet avec la sortie de l'hôte. C'est la seule transformation.
        return corps.replace("\r\n", "\n").strip("\n").strip()

    def sonde(self, attente: float = 5.0) -> bool:
        """Sonde légère : l'invité est-il à l'invite ? (une seule ligne tapée)"""
        position = self.serie.stat().st_size if self.serie.exists() else 0
        self.taper("echo TAMIS_PRET > /dev/ser1\n")
        limite = time.time() + attente
        while time.time() < limite:
            time.sleep(0.3)
            if "TAMIS_PRET" in self._serie_d(position):
                return True
        return False

    def identite(self) -> dict:
        """Identité de l'invité : noyau, version, processus."""
        return {
            "uname": self.executer("uname -a"),
            "version_tamis": self.executer("tamis version"),
        }


def main(argv: list[str]) -> int:
    if "--aide" in argv or "-h" in argv:
        print(__doc__)
        return 0
    invite = Invite()
    try:
        invite.ouvrir()
        if "--identite" in argv:
            import json
            print(json.dumps(invite.identite(), ensure_ascii=False, indent=2))
            return 0
        if not argv:
            print("KO : aucune commande", file=sys.stderr)
            return 2
        for i, commande in enumerate(argv):
            if commande == "--":
                continue
            if i:
                print("---")
            print(invite.executer(commande))
        return 0
    except Erreur as e:
        print(f"KO {e}", file=sys.stderr)
        return 1
    finally:
        invite.fermer()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
