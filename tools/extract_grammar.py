#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Extrait la grammaire du langage de politique QNX depuis les pages HTML du SDP installé.

C'est l'étape E1 du cahier des charges : la documentation en ligne renvoyait 404 pendant le
développement, la grammaire restait donc « À VÉRIFIER ». Dès que le SDP 8.0 est installé, ce
script produit un Brouillon sourcé : chaque production est remontée AVEC sa page source et
son contexte littéral, et la liste des mots-clés réellement employés (donc l'orthographe
exacte des modificateurs).

Usage :
  extract_grammar.py <chemin-doc-html> [--markdown] [--json] [--out FICHIER]

Exemples :
  extract_grammar.py ~/qnx800/8.0.0/htm/en/topic/com.qnx.doc.security.secpol --markdown
  extract_grammar.py /chemin/secpol_language.html --markdown --out /tmp/grammaire.md
"""
from __future__ import annotations

import html
import json
import pathlib
import re
import sys

# Mots-clés du langage de politique QNX (docs QNX 8.0 + relevés sur QNX 7.1 réel).
MOTS_CLES = (
    "type", "attribute", "class", "range", "default_rules",
    "allow", "allow_attach", "allow_link", "default_spawn_type",
    "derive_type", "mount_point", "include", "if", "else",
)

# Indices de modificateurs relevés dans le binaire QNX 7.1 (proof/qnx71).
MODIFICATEURS = (
    "allow", "deny", "inherit", "no-inherit", "nonroot", "root", "lock", "subrange",
)

RE_BLOC = re.compile(r"<pre\b[^>]*>(.*?)</pre>", re.S | re.I)
RE_BALISE = re.compile(r"<[^>]+>")
RE_PROD = re.compile(
    r"^(type|attribute|class|range|default_rules|allow|allow_attach|allow_link|"
    r"default_spawn_type|derive_type|mount_point|include)\s+\S")
RE_ABILITE = re.compile(r"^([A-Za-z_][A-Za-z_0-9-]*)\s*:")
RE_MODIF = re.compile(r"^([A-Za-z_][A-Za-z_0-9-]*)\s*$")


def est_debut(ligne: str) -> bool:
    """Vrai si la ligne ouvre une production (mot-clé + argument, finissant par ; ou {)."""
    l = ligne.strip()
    if not RE_PROD.match(l):
        return False
    return l.endswith(";") or l.endswith("{")


def blocs_grammaire(texte: str) -> list:
    """Relève les productions complètes : une ligne d'ouverture, puis son bloc si {."""
    lignes = texte.splitlines()
    out, i = [], 0
    while i < len(lignes):
        l = lignes[i].strip()
        if est_debut(l):
            if l.endswith("{"):
                j, prof = i, l.count("{") - l.count("}")
                while j + 1 < len(lignes) and prof > 0:
                    j += 1
                    prof += lignes[j].count("{") - lignes[j].count("}")
                out.append("\n".join(lignes[i:j + 1]))
                i = j + 1
                continue
            out.append(l)
        i += 1
    return out


def vocabulaire(blocs: list) -> tuple:
    """Relève les abilities (mot:) et les modificateurs (mot seul) dans les productions."""
    abilites, modificateurs = set(), set()
    for bloc in blocs:
        for ligne in bloc.splitlines()[1:]:
            t = ligne.strip()
            if not t or t.startswith("#"):
                continue
            m = RE_ABILITE.match(t)
            if m:
                abilites.add(m.group(1))
                continue
            m = RE_MODIF.match(t)
            if m and m.group(1).islower():
                modificateurs.add(m.group(1))
    return sorted(abilites), sorted(modificateurs)


def normaliser(ligne: str) -> str:
    return re.sub(r"[ \t]+", " ", ligne).strip()


def texte_brut(source: str) -> str:
    """ Retire les balises et décode les entités ; conserve les sauts de ligne. """
    sans = RE_BALISE.sub(" ", source)
    return html.unescape(sans)


def blocs_code(source: str) -> list:
    """ Blocs de code d'une page HTML : <pre> en priorité, sinon le texte entier. """
    found = RE_BLOC.findall(source)
    if found:
        return [html.unescape(b) for b in found]
    return [texte_brut(source)]


def extraire_page(chemin: pathlib.Path) -> dict:
    """ Relève les productions d'une page : bloc complet, page source, vocabulaire.

    Les productions viennent des blocs de code et sont capturées ENTIÈREMENT (une règle
    `allow … { … };` s'étend sur plusieurs lignes) ; le vocabulaire (abilities,
    modificateurs) est relu DANS les productions, et les mots-clés dans toute la page.
    """
    source = chemin.read_text(encoding="utf-8", errors="replace")
    productions = []
    abilites, mods = set(), set()
    for bloc in blocs_code(source):
        for texte in blocs_grammaire(bloc):
            productions.append({
                "production": texte.strip(),
                "contexte": normaliser(texte.splitlines()[0]),
                "page": chemin.name,
            })
            a, m = vocabulaire([texte])
            abilites |= set(a)
            mods |= set(m)
    pleine = texte_brut(source)
    mots = {mot for mot in MOTS_CLES if re.search(r"\b" + mot + r"\b", pleine)}
    return {"page": chemin.name, "productions": productions,
            "mots_cles": sorted(mots), "abilites": sorted(abilites),
            "modificateurs": sorted(mods)}


def balayer(chemin: pathlib.Path) -> list:
    """ Toutes les pages HTML du dossier (ou la page unique). """
    if chemin.is_file():
        return [chemin]
    return sorted(p for p in chemin.rglob("*.html"))


def markdown(reports: list, total: int) -> str:
    out = ["<!-- SPDX-License-Identifier: MIT -->",
           "# Brouillon de grammaire — extraction automatique depuis les docs du SDP",
           "",
           "> **GÉNÉRÉ automatiquement** par `tools/extract_grammar.py` depuis les pages HTML",
           "> installées avec le SDP. Chaque production est à **relire** avant de figer",
           "> `docs/GRAMMAR.md` : l'extraction est littérale, elle ne garantit pas l'exactitude",
           "> sémantique (les docs contiennent aussi des exemples et des fragments de code).",
           "",
           f"Pages analysées : **{len(reports)}** · productions relevées : **{total}**",
           ""]
    for r in reports:
        if not r["productions"]:
            continue
        out.append(f"## {r['page']}")
        out.append("")
        for p in r["productions"]:
            out.append("```")
            out.append(p["production"])
            out.append("```")
            out.append("")
    abilites = sorted({a for r in reports for a in r.get("abilites", [])})
    mods = sorted({m for r in reports for m in r.get("modificateurs", [])})
    out.append("## Vocabulaire relevé")
    out.append("")
    out.append(f"- Mots-clés vus : `{', '.join(sorted({m for r in reports for m in r['mots_cles']}))}`")
    out.append(f"- Modificateurs vus : `{', '.join(mods)}`")
    out.append(f"- Modificateurs de référence (QNX 7.1 réel) : `{', '.join(MODIFICATEURS)}`")
    out.append(f"- Abilities vues ({len(abilites)}) : `{', '.join(abilites)}`")
    out.append("")
    out.append("À comparer avec les modificateurs attestés sur QNX 7.1 "
               "(`preuve/qnx71/syntaxe-attenstee.txt`) : toute divergence est un signal fort.")
    out.append("")
    return "\n".join(out)


def main(argv: list) -> int:
    if len(argv) < 2:
        print(__doc__)
        return 2
    chemin = pathlib.Path(argv[1]).expanduser()
    if not chemin.exists():
        print(f"KO chemin introuvable : {chemin}")
        return 3
    pages = balayer(chemin)
    if not pages:
        print(f"KO aucune page HTML sous {chemin}")
        return 3
    reports = [extraire_page(p) for p in pages]
    total = sum(len(r["productions"]) for r in reports)
    if "--json" in argv:
        print(json.dumps({"pages": reports, "total": total}, ensure_ascii=False, indent=2))
        return 0
    texte = markdown(reports, total)
    if "--out" in argv:
        sortie = pathlib.Path(argv[argv.index("--out") + 1])
        sortie.write_text(texte, encoding="utf-8")
        print(f"OK brouillon écrit dans {sortie} ({total} productions, {len(reports)} pages)")
    else:
        print(texte)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
