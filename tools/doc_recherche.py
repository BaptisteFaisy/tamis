#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Cherche un terme dans la documentation QNX installée avec le SDP.

Sert à revalider les attestations relevées sur QNX 7.1 contre la doc 8.0 : pour chaque
terme, on sait dans quelles pages il apparaît et combien de fois.

Usage :
  doc_recherche.py <dossier-doc> <mot> [<mot> …]
  doc_recherche.py --extrait <dossier-doc>     # extrait les productions de grammaire
"""
from __future__ import annotations

import pathlib
import re
import sys


def occurrences(source: str, mot: str) -> int:
    motif = r"(?<![A-Za-z_])" + re.escape(mot) + r"(?![A-Za-z_])"
    return len(re.findall(motif, source))


def chercher(dossier: pathlib.Path, mots: list) -> int:
    pages = sorted(dossier.glob("*.html"))
    if not pages:
        print(f"KO aucune page HTML dans {dossier}")
        return 3
    textes = {}
    for p in pages:
        textes[p.name] = p.read_text(encoding="utf-8", errors="replace")
    total_presents = 0
    for mot in mots:
        hits = [(nom, occurrences(texte, mot)) for nom, texte in textes.items()
                if occurrences(texte, mot) > 0]
        total_presents += len(hits)
        detail = ", ".join(f"{n}({c})" for n, c in hits[:5])
        print(f"{mot:14} -> {len(hits):2d} page(s)  {detail}")
    return 0 if total_presents else 1


def main(argv: list) -> int:
    if len(argv) < 3:
        print(__doc__)
        return 2
    dossier = pathlib.Path(argv[1]).expanduser()
    if not dossier.is_dir():
        print(f"KO dossier absent : {dossier}")
        return 3
    return chercher(dossier, argv[2:])


if __name__ == "__main__":
    sys.exit(main(sys.argv))
