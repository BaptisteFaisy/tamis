#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Génère les blocs dynamiques du site depuis les sources du dépôt (une seule source de vérité).

Blocs régénérés :
  site/app.js      FIXTURES  <- fixtures/attaque.mpf, fixtures/durcie.mpf
  site/index.html  IDEES     <- docs/100-idees.md
  site/index.html  PREUVES   <- docs/VALIDATION.md

Usage :
  build_site.py                 régénère site/ en place
  build_site.py --out <dossier> copie site/ puis régénère là (contrôle de dérive en CI)
"""
from __future__ import annotations

import html
import json
import pathlib
import re
import shutil
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent


def inline(texte: str) -> str:
    """Échappe le HTML puis rend `code` et **gras** (sous-ensemble Markdown)."""
    t = html.escape(texte)
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)
    t = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", t)
    return t


def remplacer_bloc(texte: str, marqueur: str, ouv: str, ferm: str, contenu: str) -> str:
    """Remplace le bloc entre `<ouv> <marqueur>:START <ferm>` et `…:END …`."""
    debut = f"{ouv} {marqueur}:START {ferm}"
    fin = f"{ouv} {marqueur}:END {ferm}"
    pat = re.compile(re.escape(debut) + r".*?" + re.escape(fin), re.S)
    if not pat.search(texte):
        raise SystemExit(f"KO bloc introuvable : {marqueur}")
    return pat.sub(lambda _: debut + "\n" + contenu.rstrip() + "\n" + fin, texte, count=1)


def table_html(lignes: list[str]) -> str:
    """Convertit un bloc de tableau Markdown en <table>."""
    rows = [l for l in lignes if l.strip().startswith("|")]
    if len(rows) < 2:
        return ""
    cells = [
        [c.strip() for c in r.strip().strip("|").split("|")]
        for r in rows
    ]
    head, body = cells[0], cells[2:]
    out = ["<table>", "<thead><tr>"]
    out += [f"<th>{inline(c)}</th>" for c in head]
    out.append("</tr></thead><tbody>")
    for r in body:
        out.append("<tr>" + "".join(f"<td>{inline(c)}</td>" for c in r) + "</tr>")
    out.append("</tbody></table>")
    return "\n".join(out)


def section(md: str, titre: str) -> list[str]:
    """Lignes de la section dont le titre commence par `titre` (jusqu'au prochain ##)."""
    lignes = md.splitlines()
    deb = fin = None
    for i, l in enumerate(lignes):
        if l.startswith("## ") and l[3:].strip().startswith(titre):
            deb = i + 1
        elif deb is not None and l.startswith("## "):
            fin = i
            break
    return lignes[deb:(fin if fin is not None else len(lignes))]


def gen_idees() -> str:
    md = (RACINE / "docs" / "100-idees.md").read_text(encoding="utf-8")
    out = ['<div class="ideas">']
    titre = None
    items: list[str] = []
    start = 1

    def flush() -> None:
        if titre and items:
            out.append(f"<h3>{inline(titre)}</h3>")
            out.append(f'<ol start="{start}">')
            out.extend(items)
            out.append("</ol>")

    for ligne in md.splitlines():
        m_sec = re.match(r"^## ([A-J])\. (.+)$", ligne)
        m_item = re.match(r"^(\d+)\.\s+(.*)$", ligne)
        if m_sec:
            flush()
            titre, items = f"{m_sec.group(1)}. {m_sec.group(2)}", []
            continue
        if m_item and titre:
            if not items:
                start = int(m_item.group(1))
            texte = m_item.group(2)
            fait = texte.startswith("⭐")
            texte = texte.lstrip("⭐ ").strip()
            cls = ' class="done"' if fait else ""
            prefixe = "✓ " if fait else ""
            items.append(f"<li{cls}>{prefixe}{inline(texte)}</li>")
    flush()
    out.append('<p class="hint">✓ = implémenté et essayable dans le Playground. '
               "Liste complète annotée : <code>docs/100-idees.md</code>.</p>")
    out.append("</div>")
    return "\n".join(out)


def gen_preuves() -> str:
    md = (RACINE / "docs" / "VALIDATION.md").read_text(encoding="utf-8")
    out = ['<div class="prose">', "<h3>Les 8 niveaux de vérification</h3>"]
    out.append(table_html(section(md, "Les 8 niveaux")))
    out.append("<h3>État par niveau</h3>")
    out.append(table_html(section(md, "Statut")))
    out.append("<p class=\"hint\">Commandes : <code>make test</code> · <code>make sanitize</code> · "
               "<code>make mutants</code> · <code>bash tests/test_js.sh</code> · "
               "<code>bash tests/test_differentiel.sh</code></p>")
    out.append("</div>")
    return "\n".join(out)


def gen_fixtures() -> str:
    exemples = {}
    for nom in ("attaque", "durcie"):
        p = RACINE / "fixtures" / f"{nom}.mpf"
        exemples[nom] = p.read_text(encoding="utf-8")
    return "const EXAMPLES = " + json.dumps(exemples, ensure_ascii=False) + ";"


def generer(cible: pathlib.Path) -> None:
    app = (cible / "app.js").read_text(encoding="utf-8")
    app = remplacer_bloc(app, "FIXTURES", "/*", "*/", gen_fixtures())
    (cible / "app.js").write_text(app, encoding="utf-8")

    idx = (cible / "index.html").read_text(encoding="utf-8")
    idx = remplacer_bloc(idx, "IDEES", "<!--", "-->", gen_idees())
    idx = remplacer_bloc(idx, "PREUVES", "<!--", "-->", gen_preuves())
    (cible / "index.html").write_text(idx, encoding="utf-8")


def main(argv: list[str]) -> int:
    if "--out" in argv:
        out = pathlib.Path(argv[argv.index("--out") + 1])
        if out.exists():
            shutil.rmtree(out)
        shutil.copytree(RACINE / "site", out)
        generer(out)
        print(f"OK site régénéré dans {out}")
    else:
        generer(RACINE / "site")
        print("OK site/ régénéré depuis fixtures/ et docs/")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
