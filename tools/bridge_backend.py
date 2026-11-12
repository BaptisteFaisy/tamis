#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Opérations du pont local Tamis : le VRAI cœur C et le VRAI banc QNX.

Aucune logique HTTP ici (voir tools/tamis-bridge.py). Aucune commande n'est construite par
concaténation : tout passe par des listes d'arguments (subprocess sans shell), et les seules
commandes exécutées sur la VM sont des préréglages figés ci-dessous.
"""
from __future__ import annotations

import os
import re
import shlex
import subprocess
import tempfile
import time
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent
CLI = RACINE / "build" / "tamis"

CMDS = ("lint", "reach", "proof", "explain", "minimize", "diff", "view")
FORMATS = ("text", "json", "md")
PAIR_CMDS = ("reach", "proof", "explain")
SAFE_NAME = re.compile(r"^[A-Za-z0-9_.\-]{1,128}$")
MAX_POLICY = 512 * 1024

# Préréglages FIGÉS : libellé -> commande exécutée dans l'invité QNX.
# Le navigateur choisit un nom, jamais une commande.
PRESETS = {
    "uname": ("Version du système QNX", "uname -a"),
    "boot": ("Contenu de /proc/boot (secpol, ability)",
             "ls -la /proc/boot/ | grep -iE 'secpol|ability'"),
    "secpol": ("Outil officiel secpol (visualiseur du blob)", "secpol 2>&1; echo rc=$?"),
    "secpolmonitor": ("Outil officiel secpolmonitor", "secpolmonitor 2>&1; echo rc=$?"),
    "libsecpol": ("Bibliothèque libsecpol présente", "ls -la /proc/boot/libsecpol*"),
    "hosttools": ("Outils hôtes (SDP) absents de la cible",
                  "for b in secpolcompile secpolgenerate secpolpush; do "
                  "command -v $b || echo \"$b: absent (outil hôte SDP)\"; done"),
    "procnto": ("Noyau en cours (politique chargée ?)", "pidin -F '%a %n' | head -5"),
}


def vm_dir() -> Path | None:
    """Localise le banc QNX : variable TAMIS_QNX_VM puis emplacements usuels."""
    cands = [os.environ.get("TAMIS_QNX_VM"),
             str(Path.home() / "Bureau" / "qnx71-vm"),
             str(Path.home() / "qnx71-vm")]
    for c in cands:
        if c and (Path(c) / "qnx-vm.sh").is_file():
            return Path(c)
    return None


def run(argv: list, cwd=None, timeout: int = 120) -> dict:
    """Exécute sans shell, capture sortie et code, ne lève jamais d'exception."""
    try:
        p = subprocess.run(argv, cwd=str(cwd) if cwd else None, capture_output=True,
                           text=True, timeout=timeout)
        return {"exit": p.returncode, "stdout": p.stdout, "stderr": p.stderr}
    except FileNotFoundError as exc:
        return {"exit": 127, "stdout": "", "stderr": f"commande introuvable : {exc}"}
    except subprocess.TimeoutExpired:
        return {"exit": 124, "stdout": "", "stderr": f"délai dépassé ({timeout} s)"}


def ensure_cli() -> bool:
    if CLI.is_file():
        return True
    return run(["make", "-s", "cli"], cwd=RACINE, timeout=240)["exit"] == 0 and CLI.is_file()


def run_tamis(cmd: str, policy: str, policy2: str | None = None,
              src: str | None = None, dst: str | None = None, fmt: str = "text",
              label: str = "policy.mpf") -> dict:
    """Exécute le VRAI binaire C sur une politique écrite dans un fichier temporaire.

    `label` est le nom rapporté dans les verdicts : le chemin temporaire réel ne fuit pas
    dans la sortie, ce qui rend les rendus stables et comparables (site ↔ CLI).
    """
    if cmd not in CMDS:
        return {"error": f"commande refusée (hors liste blanche) : {cmd}"}
    if fmt not in FORMATS:
        return {"error": f"format refusé : {fmt}"}
    if len(policy or "") > MAX_POLICY or len(policy2 or "") > MAX_POLICY:
        return {"error": "politique trop volumineuse"}
    for nom, val in (("src", src), ("dst", dst), ("label", label)):
        if val is not None and not SAFE_NAME.match(val):
            return {"error": f"{nom} invalide (caractères autorisés : A-Za-z0-9_.-)"}
    if not ensure_cli():
        return {"error": "build/tamis absent et 'make cli' a échoué"}
    if cmd in PAIR_CMDS and not (src and dst):
        return {"error": f"{cmd} exige src et dst"}
    if cmd == "diff" and policy2 is None:
        return {"error": "diff exige une seconde politique"}

    with tempfile.TemporaryDirectory(prefix="tamis-bridge-") as d:
        f1 = Path(d) / "policy.mpf"
        f1.write_text(policy or "", encoding="utf-8")
        argv = [str(CLI), cmd, str(f1)]
        affiche = [CLI.name, cmd, "<politique>"]
        if cmd == "diff":
            f2 = Path(d) / "candidate.mpf"
            f2.write_text(policy2 or "", encoding="utf-8")
            argv.append(str(f2))
            affiche.append("<candidate>")
        if cmd in PAIR_CMDS:
            argv += ["--src", src, "--dst", dst]
            affiche += ["--src", src, "--dst", dst]
        argv += ["--format", fmt, "--label", label]
        affiche += ["--format", fmt, "--label", label]
        res = run(argv, cwd=RACINE, timeout=120)
        res["argv"] = " ".join(affiche)
        res["engine"] = "core C (build/tamis)"
        return res


QNX_HOST = os.environ.get("TAMIS_QNX_HOST", "").strip()
QNX_DIR = os.environ.get("TAMIS_QNX_DIR", "~/qnx71-vm")


def banc_dispo() -> bool:
    """Vrai si un banc QNX est joignable : distant (TAMIS_QNX_HOST) ou local."""
    return bool(QNX_HOST) or vm_dir() is not None


def banc_label() -> str:
    if QNX_HOST:
        return f"distant ({QNX_HOST}:{QNX_DIR})"
    d = vm_dir()
    return f"local ({d})" if d else "non configuré"


def _dir_distant() -> str:
    """Expression shell du dossier distant, avec $HOME développé mais le reste cité."""
    d = QNX_DIR
    if d.startswith("~/"):
        return '"$HOME"/' + shlex.quote(d[2:])
    if d == "~":
        return '"$HOME"'
    return shlex.quote(d)


def _harness(args: list, timeout: int) -> dict:
    """Exécute le harnais du banc : localement, ou sur TAMIS_QNX_HOST via ssh.

    `args[0]` est le nom du script du banc, le reste ses arguments. Les arguments
    viennent exclusivement de listes blanches internes ; ils sont passés en argv (jamais
    de shell côté local) et cités un par un pour le shell distant.
    """
    if QNX_HOST:
        remote = f"{_dir_distant()}/{shlex.quote(args[0])} " + \
                 " ".join(shlex.quote(a) for a in args[1:])
        return run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=10", QNX_HOST, remote],
                   timeout=timeout)
    d = vm_dir()
    if d is None:
        return {"exit": 3, "stdout": "",
                "stderr": "banc QNX non configuré (TAMIS_QNX_VM ou TAMIS_QNX_HOST)"}
    return run(["./" + args[0]] + list(args[1:]), cwd=d, timeout=timeout)


def _ssh_pret(timeout: int = 25) -> bool:
    return "PRET" in _harness(["qnx-ssh", "echo PRET"], timeout)["stdout"]


def qnx_status() -> dict:
    if not banc_dispo():
        return {"configured": False, "banc": banc_label(),
                "hint": "définir TAMIS_QNX_VM (banc local) ou TAMIS_QNX_HOST (banc distant)"}
    st = _harness(["qnx-vm.sh", "status"], 25)
    running = "en marche" in st["stdout"]
    return {"configured": True, "banc": banc_label(), "running": running,
            "ssh_ready": _ssh_pret() if running else False,
            "status": st["stdout"].strip(), "stderr": st.get("stderr", "")}


def qnx_boot(wait_s: int = 300) -> dict:
    if not banc_dispo():
        return {"error": "banc QNX non configuré (TAMIS_QNX_VM ou TAMIS_QNX_HOST)"}
    out = _harness(["qnx-vm.sh", "start"], 120)
    t0 = time.time()
    while time.time() - t0 < wait_s:
        if _ssh_pret():
            return {"ok": True, "boot": out["stdout"].strip(),
                    "elapsed_s": round(time.time() - t0, 1), "banc": banc_label()}
        time.sleep(10)
    return {"ok": False, "boot": (out["stdout"] + out.get("stderr", "")).strip(),
            "error": f"SSH non prêt après {wait_s} s", "banc": banc_label()}


def qnx_stop() -> dict:
    if not banc_dispo():
        return {"error": "banc QNX non configuré (TAMIS_QNX_VM ou TAMIS_QNX_HOST)"}
    return _harness(["qnx-vm.sh", "stop"], 60)


def qnx_exec(preset: str) -> dict:
    # La liste blanche est vérifiée AVANT toute autre chose : une commande inconnue est
    # refusée quel que soit l'état du banc (et sans révéler cet état).
    if preset not in PRESETS:
        return {"error": f"préréglage refusé (hors liste blanche) : {preset}"}
    if not banc_dispo():
        return {"error": "banc QNX non configuré (TAMIS_QNX_VM ou TAMIS_QNX_HOST)"}
    label, cmd = PRESETS[preset]
    res = _harness(["qnx-ssh", cmd], 90)
    res.update(preset=preset, label=label, remote_cmd=cmd, banc=banc_label())
    return res
