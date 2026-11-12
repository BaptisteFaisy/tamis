#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Génère le site bilingue depuis ses sources — une seule source de vérité, aucune dérive.

Sources :
  site/content.json    fonctionnalités, exigences, soumission, codelab, preuves EN, vidéo
  docs/100-idees.md    les 100 idées (FR)
  docs/VALIDATION.md   les niveaux de vérification (FR)
  fixtures/*.mpf       les politiques d'exemple du playground

Usage :
  build_site.py                  régénère site/ en place
  build_site.py --out <dossier>  copie site/ puis régénère là (contrôle de dérive en CI)
"""
from __future__ import annotations

import html
import json
import pathlib
import re
import shutil
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
SITE = RACINE / "site"


def load_content() -> dict:
    return json.loads((SITE / "content.json").read_text(encoding="utf-8"))


def inline(texte: str) -> str:
    """Échappe le HTML puis rend `code` et **gras** (sous-ensemble Markdown)."""
    t = html.escape(str(texte))
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)
    return re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", t)


def raw(texte: str) -> str:
    """Contenu déjà en HTML (champs *_en pré-échappés de content.json)."""
    return str(texte)


def bi(fr: str, en: str) -> str:
    """Deux blocs frères, l'un masqué selon la langue active (CSS)."""
    return f'<div data-lang="fr">{fr}</div>\n<div data-lang="en">{en}</div>'


def td(contenu: str, cls: str = "") -> str:
    return f'<td class="{cls}">{contenu}</td>' if cls else f"<td>{contenu}</td>"


def table(heads: list, rows: list, cls: str = "") -> str:
    out = [f'<table class="{cls}">' if cls else "<table>", "<thead><tr>"]
    out += [f"<th>{h}</th>" for h in heads]
    out.append("</tr></thead><tbody>")
    for cells in rows:
        out.append("<tr>" + "".join(cells) + "</tr>")
    out.append("</tbody></table>")
    return "\n".join(out)


def list_html(items: list, ordered: bool = False) -> str:
    tag = "ol" if ordered else "ul"
    return f"<{tag}>" + "".join(f"<li>{inline(i)}</li>" for i in items) + f"</{tag}>"


def paras(items: list) -> str:
    return "".join(f"<p>{inline(p)}</p>" for p in items)


def remplacer_bloc(texte: str, marqueur: str, contenu: str) -> str:
    debut, fin = f"<!-- {marqueur}:START -->", f"<!-- {marqueur}:END -->"
    pat = re.compile(re.escape(debut) + r".*?" + re.escape(fin), re.S)
    if not pat.search(texte):
        raise SystemExit(f"KO bloc introuvable : {marqueur}")
    return pat.sub(lambda _: debut + "\n" + contenu.rstrip() + "\n" + fin, texte, count=1)


def md_table_html(lignes: list) -> str:
    rows = [l for l in lignes if l.strip().startswith("|")]
    if len(rows) < 3:
        return ""
    cells = [[c.strip() for c in r.strip().strip("|").split("|")] for r in rows]
    return table([inline(c) for c in cells[0]],
                 [[td(inline(c)) for c in r] for r in cells[2:]])


def md_section(md: str, titre: str) -> list:
    lignes = md.splitlines()
    deb = fin = None
    for i, l in enumerate(lignes):
        if l.startswith("## ") and l[3:].strip().startswith(titre):
            deb = i + 1
        elif deb is not None and l.startswith("## "):
            fin = i
            break
    return lignes[deb:(fin if fin is not None else len(lignes))]


# ————————————————————————— blocs générés —————————————————————————

def gen_features(c: dict) -> str:
    st = c["status"]
    rows_fr, rows_en = [], []
    for f in c["features"]:
        cls = {"done": "ok", "ready": "wait", "blocked": "wait", "todo": "wait"}[f["st"]]
        rows_fr.append([td(f"<b>{inline(f['fr'])}</b>"), td(f"<code>{inline(f['cmd'])}</code>"),
                        td(inline(f["wfr"])), td(inline(f["req"])), td(st[f["st"]]["fr"], cls)])
        rows_en.append([td(f"<b>{inline(f['en'])}</b>"), td(f"<code>{inline(f['cmd'])}</code>"),
                        td(inline(f["wen"])), td(inline(f["req"])), td(st[f["st"]]["en"], cls)])
    hfr = ["Fonctionnalité", "Commande", "Ce que ça apporte", "Exigence", "État"]
    hen = ["Feature", "Command", "What it adds", "Requirement", "Status"]
    return bi(table(hfr, rows_fr, "feat"), table(hen, rows_en, "feat"))


def gen_requirements(c: dict) -> str:
    st = c["req_status"]
    rows_fr, rows_en = [], []
    for r in c["requirements"]:
        cls = "ok" if r["st"] == "done" else "wait"
        head = [td(f"<b>{r['id']}</b>"), td(r["lv"])]
        rows_fr.append(head + [td(inline(r["fr"])), td(st[r["st"]]["fr"], cls)])
        rows_en.append(head + [td(inline(r["en"])), td(st[r["st"]]["en"], cls)])
    return bi(table(["ID", "Niv.", "Exigence", "État"], rows_fr, "feat"),
              table(["ID", "Lvl", "Requirement", "Status"], rows_en, "feat"))


def gen_preuves(c: dict) -> str:
    md = (RACINE / "docs" / "VALIDATION.md").read_text(encoding="utf-8")
    fr = ['<div class="prose">', "<h3>Les 8 niveaux de vérification</h3>",
          md_table_html(md_section(md, "Les 8 niveaux")),
          "<h3>État par niveau</h3>", md_table_html(md_section(md, "Statut")), "</div>"]
    rows = [[td(str(p["n"])), td(f"<b>{raw(p['what'])}</b>"), td(raw(p["how"])), td(raw(p["target"]))]
            for p in c["proofs_en"]]
    en = ['<div class="prose">', "<h3>The 8 verification levels</h3>",
          table(["#", "Level", "How", "Result"], rows), "</div>"]
    return bi("\n".join(fr), "\n".join(en))


def gen_idees_fr() -> str:
    md = (RACINE / "docs" / "100-idees.md").read_text(encoding="utf-8")
    out, titre, items, start = ['<div class="ideas">'], None, [], 1

    def flush():
        if titre and items:
            out.append(f"<h3>{inline(titre)}</h3>")
            out.append(f'<ol start="{start}">' + "".join(items) + "</ol>")

    for ligne in md.splitlines():
        m_sec = re.match(r"^## ([A-J])\. (.+)$", ligne)
        m_item = re.match(r"^(\d+)\.\s+(.*)$", ligne)
        if m_sec:
            flush()
            titre, items = f"{m_sec.group(1)}. {m_sec.group(2)}", []
        elif m_item and titre:
            if not items:
                start = int(m_item.group(1))
            texte = m_item.group(2)
            fait = texte.startswith("⭐")
            cls = ' class="done"' if fait else ""
            items.append(f"<li{cls}>{'✓ ' if fait else ''}{inline(texte.lstrip('⭐ ').strip())}</li>")
    flush()
    out.append('<p class="hint">✓ = implémenté et essayable dans le Playground.</p></div>')
    return "\n".join(out)


def gen_idees_en(c: dict) -> str:
    done = [f for f in c["features"] if f["st"] == "done"]
    out = ['<div class="ideas">',
           "<p>The full annotated 100-idea roadmap is maintained in French in "
           "<code>docs/100-idees.md</code> (switch the site to FR to read it). "
           "It is grouped in ten families:</p>", "<ul>"]
    out += [f"<li><b>{k}.</b> {v}</li>" for k, v in sorted(c["idea_categories_en"].items())]
    out.append("</ul>")
    out.append(f"<h3>The {len(done)} ideas already shipped</h3><ol>")
    out += [f"<li class='done'>✓ {inline(f['en'])} — <code>{inline(f['cmd'])}</code></li>" for f in done]
    out.append("</ol></div>")
    return "\n".join(out)


def gen_idees(c: dict) -> str:
    return bi(gen_idees_fr(), gen_idees_en(c))


def _sub_head(s: dict) -> str:
    fr = (f'<div class="subcard"><h2>{inline(s["name"])}</h2>'
          f'<p class="tagline">{inline(s["tagline_fr"])}</p>'
          f'<p><b>Catégorie :</b> {inline(s["category_fr"])}</p>'
          f'<p><b>Bonus visés :</b></p>{list_html(s["bonuses_fr"])}</div>')
    en = (f'<div class="subcard"><h2>{inline(s["name"])}</h2>'
          f'<p class="tagline">{inline(s["tagline_en"])}</p>'
          f'<p><b>Category:</b> {inline(s["category_en"])}</p>'
          f'<p><b>Bonuses targeted:</b></p>{list_html(s["bonuses_en"])}</div>')
    return bi(fr, en)


def _sub_video(c: dict) -> str:
    rows_fr = [[td(v["t"]), td(inline(v["fr"]))] for v in c["video"]]
    rows_en = [[td(v["t"]), td(inline(v["en"]))] for v in c["video"]]
    return bi("<h3>Script vidéo (3 minutes)</h3>" + table(["Temps", "Plan"], rows_fr),
              "<h3>Video script (3 minutes)</h3>" + table(["Time", "Shot"], rows_en))


def _sub_links() -> str:
    rows = [
        ["Dépôt public", "Public repository", "https://github.com/BaptisteFaisy/tamis"],
        ["Ce site", "This site", "https://baptistefaisy.github.io/tamis/"],
        ["Contrat gelé", "Frozen contract", "https://github.com/BaptisteFaisy/tamis/blob/main/CONTRACT.md"],
        ["Codelab", "Codelab",
         "https://github.com/BaptisteFaisy/tamis/blob/main/docs/codelab/01-auditer-une-politique.md"],
        ["Grammaire relevée", "Recorded grammar",
         "https://github.com/BaptisteFaisy/tamis/blob/main/docs/GRAMMAR.md"],
        ["100 idées", "100 ideas", "https://github.com/BaptisteFaisy/tamis/blob/main/docs/100-idees.md"],
    ]
    items = "".join(
        f'<li><span data-lang="fr">{inline(a)}</span><span data-lang="en">{inline(b)}</span>'
        f' : <a href="{u}">{u}</a></li>'
        for a, b, u in rows)
    return ('<h3><span data-lang="fr">Liens</span><span data-lang="en">Links</span></h3>'
            f'<ul class="links">{items}</ul>')


def gen_submission(c: dict) -> str:
    s = c["submission"]
    return "\n".join([
        _sub_head(s),
        bi('<h3>Le pitch, en une phrase</h3><p class="pitch">' + inline(s["pitch_fr"]) + "</p>",
           '<h3>The pitch, in one sentence</h3><p class="pitch">' + inline(s["pitch_en"]) + "</p>"),
        bi("<h3>Ce que fait Tamis</h3>" + paras(s["does_fr"]),
           "<h3>What Tamis does</h3>" + paras(s["does_en"])),
        bi("<h3>Preuve d'usage de QNX</h3>" + list_html(s["qnx_fr"]),
           "<h3>Proof of QNX usage</h3>" + list_html(s["qnx_en"])),
        bi("<h3>Comment c'est construit</h3>" + list_html(s["built_fr"]),
           "<h3>How it was built</h3>" + list_html(s["built_en"])),
        bi("<h3>Instructions pour les juges</h3>" + list_html(s["judges_fr"], True),
           "<h3>Testing instructions for judges</h3>" + list_html(s["judges_en"], True)),
        _sub_video(c),
        _sub_links(),
    ])


def _codelab_step(i: int, st: dict) -> str:
    cmds = (f'<pre class="cmd">{html.escape(st["cmd"])}</pre>'
            f'<pre class="out">{html.escape(st["out"])}</pre>')
    return bi(f'<h4>{i}. {inline(st["tfr"])}</h4><p>{inline(st["dfr"])}</p>{cmds}',
              f'<h4>{i}. {inline(st["ten"])}</h4><p>{inline(st["den"])}</p>{cmds}')


def gen_codelab(c: dict) -> str:
    cl = c["codelab"]
    out = [bi(f'<h2>Codelab — {inline(cl["title_fr"])}</h2><p class="lead">{inline(cl["intro_fr"])}</p>',
              f'<h2>Codelab — {inline(cl["title_en"])}</h2><p class="lead">{inline(cl["intro_en"])}</p>'),
           '<div class="steps">']
    for i, st in enumerate(cl["steps"], 1):
        out.append(f'<div class="step">{_codelab_step(i, st)}</div>')
    out.append("</div>")
    out.append(bi(f'<div class="panel"><h3>Angles morts déclarés</h3><p>{inline(cl["blind_fr"])}</p></div>',
                  f'<div class="panel"><h3>Declared blind spots</h3><p>{inline(cl["blind_en"])}</p></div>'))
    return "\n".join(out)


def gen_fixtures() -> str:
    ex = {n: (RACINE / "fixtures" / f"{n}.mpf").read_text(encoding="utf-8")
          for n in ("attaque", "durcie")}
    return "const EXAMPLES = " + json.dumps(ex, ensure_ascii=False) + ";"


def generer(cible: pathlib.Path) -> None:
    c = load_content()
    app = (cible / "app.js").read_text(encoding="utf-8")
    app = re.sub(r"/\* FIXTURES:START \*/.*?/\* FIXTURES:END \*/",
                 lambda _: "/* FIXTURES:START */\n" + gen_fixtures() + "\n/* FIXTURES:END */",
                 app, count=1, flags=re.S)
    (cible / "app.js").write_text(app, encoding="utf-8")

    idx = (cible / "index.html").read_text(encoding="utf-8")
    for marqueur, contenu in (("FEATURES", gen_features(c)), ("REQUIREMENTS", gen_requirements(c)),
                              ("PREUVES", gen_preuves(c)), ("IDEES", gen_idees(c)),
                              ("SUBMISSION", gen_submission(c)), ("CODELAB", gen_codelab(c))):
        idx = remplacer_bloc(idx, marqueur, contenu)
    (cible / "index.html").write_text(idx, encoding="utf-8")


def main(argv: list) -> int:
    if "--out" in argv:
        out = pathlib.Path(argv[argv.index("--out") + 1])
        if out.exists():
            shutil.rmtree(out)
        shutil.copytree(SITE, out)
        generer(out)
        print(f"OK site régénéré dans {out}")
    else:
        generer(SITE)
        print("OK site/ régénéré depuis content.json, fixtures/ et docs/")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
