#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Garde de taille : < 500 lignes/fichier, < 50 lignes/fonction (règle user 2026-10-24)."""
from __future__ import annotations
import pathlib
import re
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
MAX_FICHIER = 500
MAX_FONCTION = 50
SUFFIXES = {".sh", ".py", ".c", ".h"}
IGNORE = {".git", "artefacts", "__pycache__", "fixtures", "build"}
RE_DEF_PY = re.compile(r"^\s*(?:async\s+)?def\s+(\w+)")
RE_DEF_C = re.compile(r"^[A-Za-z_][\w \t\*]*?\b(\w+)\s*\([^;]*\)\s*\{?\s*$")


def fichiers():
    for chemin in sorted(RACINE.rglob("*")):
        if not chemin.is_file() or chemin.suffix not in SUFFIXES:
            continue
        if IGNORE & set(chemin.relative_to(RACINE).parts[:-1]):
            continue
        yield chemin


def fonctions_python(lignes):
    deb, nom = None, None
    for i, ligne in enumerate(lignes):
        m = RE_DEF_PY.match(ligne)
        if m:
            if deb is not None:
                yield nom, deb, i - deb
            nom, deb = m.group(1), i
    if deb is not None:
        yield nom, deb, len(lignes) - deb


def fonctions_c(lignes):
    prof, deb, nom = 0, None, None
    for i, ligne in enumerate(lignes):
        if prof == 0:
            m = RE_DEF_C.match(ligne)
            if m and not ligne.lstrip().startswith(("#", "//", "*")):
                nom, deb = m.group(1), i
        prof += ligne.count("{") - ligne.count("}")
        if prof <= 0 and deb is not None and "{" in "".join(lignes[deb:i + 1]):
            yield nom, deb, i - deb + 1
            prof, deb, nom = 0, None, None


def verifier() -> int:
    echecs = []
    n_fichiers = 0
    for chemin in fichiers():
        n_fichiers += 1
        try:
            lignes = chemin.read_text(encoding="utf-8").splitlines()
        except UnicodeDecodeError:
            continue
        if len(lignes) > MAX_FICHIER:
            echecs.append(f"{chemin.relative_to(RACINE)} : {len(lignes)} lignes > {MAX_FICHIER}")
        gen = fonctions_python if chemin.suffix == ".py" else fonctions_c
        if chemin.suffix in {".c", ".h", ".py"}:
            for nom, deb, taille in gen(lignes):
                if taille > MAX_FONCTION:
                    echecs.append(
                        f"{chemin.relative_to(RACINE)}:{deb + 1} {nom}() : "
                        f"{taille} lignes > {MAX_FONCTION}")
    for e in echecs:
        print(f"KO   {e}")
    if echecs:
        print(f"FAILED {len(echecs)} dépassement(s)")
        return 1
    print(f"OK 0 dépassement sur {n_fichiers} fichier(s)")
    return 0


if __name__ == "__main__":
    sys.exit(verifier())
