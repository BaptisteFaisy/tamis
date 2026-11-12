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


def qnx_status() -> dict:
    d = vm_dir()
    if d is None:
        return {"configured": False,
                "hint": "définir TAMIS_QNX_VM vers un dossier contenant qnx-vm.sh"}
    st = run(["./qnx-vm.sh", "status"], cwd=d, timeout=20)
    running = "en marche" in st["stdout"]
    ssh = False
    if running:
        ssh = "PRET" in run(["./qnx-ssh", "echo PRET"], cwd=d, timeout=25)["stdout"]
    return {"configured": True, "dir": str(d), "running": running,
            "ssh_ready": ssh, "status": st["stdout"].strip()}


def qnx_boot(wait_s: int = 180) -> dict:
    d = vm_dir()
    if d is None:
        return {"error": "banc QNX non configuré (TAMIS_QNX_VM)"}
    out = run(["./qnx-vm.sh", "start"], cwd=d, timeout=90)
    t0 = time.time()
    while time.time() - t0 < wait_s:
        if "PRET" in run(["./qnx-ssh", "echo PRET"], cwd=d, timeout=25)["stdout"]:
            return {"ok": True, "boot": out["stdout"].strip(),
                    "elapsed_s": round(time.time() - t0, 1)}
        time.sleep(10)
    return {"ok": False, "boot": out["stdout"].strip(),
            "error": f"SSH non prêt après {wait_s} s"}


def qnx_stop() -> dict:
    d = vm_dir()
    if d is None:
        return {"error": "banc QNX non configuré (TAMIS_QNX_VM)"}
    return run(["./qnx-vm.sh", "stop"], cwd=d, timeout=40)


def qnx_exec(preset: str) -> dict:
    # La liste blanche est vérifiée AVANT toute autre chose : une commande inconnue est
    # refusée quel que soit l'état du banc (et sans révéler cet état).
    if preset not in PRESETS:
        return {"error": f"préréglage refusé (hors liste blanche) : {preset}"}
    d = vm_dir()
    if d is None:
        return {"error": "banc QNX non configuré (TAMIS_QNX_VM)"}
    label, cmd = PRESETS[preset]
    res = run(["./qnx-ssh", cmd], cwd=d, timeout=90)
    res.update(preset=preset, label=label, remote_cmd=cmd)
    return res
