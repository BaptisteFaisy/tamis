#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fusionne les lots d'idées de consolidation en un catalogue unique, sans doublon.

Les lots sont produits par theme (docs/idees/NN-theme.md, 100 idées chacun). Ce script les
concatène, retire les doublons exacts (comparaison normalisée), renumérote et écrit :
  - docs/1000-idees-consolidation.md  : le catalogue, avec sommaire par thème
  - docs/1000-idees-consolidation.json : la même chose, exploitable par le site

Usage : merge_idees.py [--racine DOSSIER] [--json]
"""
from __future__ import annotations

import json
import pathlib
import re
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
LOTS = RACINE / "docs" / "idees"
SORTIE_MD = RACINE / "docs" / "1000-idees-consolidation.md"
SORTIE_JSON = RACINE / "docs" / "1000-idees-consolidation.json"
RE_IDEE = re.compile(r"^\s*(\d+)\.\s+(.*\S)\s*$")


def normaliser(texte: str) -> str:
    """Clé de comparaison : minuscules, sans accents ni ponctuation superflue."""
    t = texte.lower()
    t = re.sub(r"[«»\"'`,;:!?()\[\]]", " ", t)
    t = re.sub(r"\s+", " ", t).strip()
    return t


def lire_lot(chemin: pathlib.Path) -> tuple:
    """(thème, [(numéro, texte)]) pour un fichier de lot."""
    theme = "thème"
    idees = []
    for ligne in chemin.read_text(encoding="utf-8").splitlines():
        if ligne.startswith("# "):
            nom = ligne[2:].strip()
            theme = re.sub(r"^Idées de consolidation\s*[—-]\s*", "", nom)
            theme = re.sub(r"\s*\(\d+\)\s*$", "", theme).strip()
        m = RE_IDEE.match(ligne)
        if m:
            idees.append((int(m.group(1)), m.group(2)))
    return theme, idees


def collecter(lots: list) -> tuple:
    """Parcourt les lots et renvoie (catalogue, vues par thème) sans doublon."""
    catalogue, vues, deja_vu = [], {}, set()
    for chemin in lots:
        theme, idees = lire_lot(chemin)
        vues.setdefault(theme, [])
        for numero, texte in idees:
            cle = normaliser(texte)
            if cle in deja_vu:
                continue
            deja_vu.add(cle)
            vues[theme].append(texte)
            catalogue.append({"theme": theme, "texte": texte, "lot": chemin.name})
    return catalogue, vues


def ecrire(catalogue: list, vues: dict, sortie_md, sortie_json) -> None:
    """Écrit le catalogue Markdown et sa version JSON."""
    lignes = ["<!-- SPDX-License-Identifier: MIT -->",
              "# 1000 idées de consolidation — catalogue",
              "",
              f"**{len(catalogue)} idées** réparties en **{len(vues)} thèmes**, sans doublon.",
              "Généré par `tools/merge_idees.py` à partir des lots `docs/idees/*.md`.",
              "",
              "## Sommaire", ""]
    for theme, idees in vues.items():
        lignes.append(f"- **{theme}** — {len(idees)} idées")
    lignes.append("")
    index = 0
    for theme, idees in vues.items():
        lignes.append(f"## {theme}")
        lignes.append("")
        for texte in idees:
            index += 1
            lignes.append(f"{index}. {texte}")
        lignes.append("")
    sortie_md.write_text("\n".join(lignes), encoding="utf-8")
    sortie_json.write_text(
        json.dumps(catalogue, ensure_ascii=False, indent=1), encoding="utf-8")


def main(argv: list) -> int:
    racine = RACINE / "docs" / "idees"
    if "--racine" in argv:
        racine = pathlib.Path(argv[argv.index("--racine") + 1])
    if not racine.is_dir():
        print(f"KO dossier de lots absent : {racine}")
        return 3
    sortie_md = racine.parent / "1000-idees-consolidation.md"
    sortie_json = racine.parent / "1000-idees-consolidation.json"
    lots = sorted(racine.glob("*.md"))
    if not lots:
        print(f"KO aucun lot dans {racine}")
        return 3
    catalogue, vues = collecter(lots)
    ecrire(catalogue, vues, sortie_md, sortie_json)
    print(f"OK {len(catalogue)} idées uniques, {len(vues)} thèmes → {sortie_md.name}")
    for theme, idees in vues.items():
        print(f"   {theme} : {len(idees)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
