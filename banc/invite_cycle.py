#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Cycle de vie de l'invité QNX 8.0 : le réveiller, attendre qu'il réponde.

**Un seul propriétaire de la politique** : la vitrine (`tamis-web.service`, port 8092)
gère la vie de l'invité — elle l'arrête après quelques minutes d'inactivité (1,5 Go de
RAM et du CPU pour rien) et le relance à la demande. Le service de preuves ne fait donc
que **demander** à la vitrine un réveil (une visite vaut un réveil) ; il ne se rabat sur
systemd que si la vitrine ne répond pas.

Fonctions :
    reveiller()   → réveille l'invité (vitrine d'abord, systemd en filet)
    pret()        → l'invité est-il démarré et prêt à répondre ?
    attendre()    → rend un `Invite` prête à répondre, ou None si le délai passe
    etat()        → état lisible de l'invité (pour la route /etat)
"""
from __future__ import annotations

import os
import subprocess
import sys
import time
import urllib.error
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from emulateur import Invite, Erreur, SERIE  # noqa: E402

UNITE = os.environ.get("TAMIS_QNX_UNITE", "tamis-qemu8.service")
VITRINE = os.environ.get("TAMIS_VITRINE_URL", "http://127.0.0.1:8092")
DELAI_DEMARRAGE = float(os.environ.get("TAMIS_QNX_BOOT_S", "90"))
PAS = 3.0
MARQUEUR_BOOT = "RESUME"      # fin du scénario de démarrage (image/demo.sh)


def _systemctl(args: list[str], timeout: int = 20) -> bool:
    """Appelle `systemctl --user` ; True si le code de sortie est 0."""
    try:
        proc = subprocess.run(
            ["systemctl", "--user"] + args,
            capture_output=True, text=True, timeout=timeout,
        )
    except (OSError, subprocess.SubprocessError):
        return False
    return proc.returncode == 0


def unite_active() -> bool:
    """L'unité systemd de l'invité est-elle en marche ?"""
    return _systemctl(["is-active", "--quiet", UNITE], timeout=10)


def reveiller() -> bool:
    """Réveille l'invité. La vitrine est le propriétaire de la politique ; systemd n'est
    qu'un filet quand la vitrine ne répond pas."""
    try:
        with urllib.request.urlopen(VITRINE + "/", timeout=5) as rep:
            rep.read(64)
        return True
    except (OSError, urllib.error.URLError, ValueError):
        pass
    return _systemctl(["start", UNITE], timeout=30)


def pret() -> bool:
    """L'invité a fini de démarrer (le scénario de démarrage a rendu la main) ?"""
    if not unite_active():
        return False
    try:
        with open(SERIE, "rb") as f:
            contenu = f.read()
    except OSError:
        return False
    return MARQUEUR_BOOT.encode("utf-8") in contenu


def attendre(delai: float = DELAI_DEMARRAGE) -> Invite | None:
    """Attend que l'invité réponde. Rend l'`Invite` prête, ou None si le délai passe.

    On attend d'abord la fin du scénario de démarrage (`pret`) : taper dans la console
    pendant le boot mettrait des commandes en file d'attente dans l'invité.
    """
    fin = time.time() + delai
    while True:
        if pret():
            invite = Invite()
            try:
                invite.ouvrir()
                if invite.sonde():
                    return invite
            except Erreur:
                pass
            invite.fermer()
        if time.time() >= fin:
            return None
        time.sleep(PAS)


def etat() -> dict:
    """État lisible de l'invité, sans toucher à la console."""
    return {
        "unite": UNITE,
        "actif": unite_active(),
        "pret": pret(),
        "vitrine": VITRINE,
    }
