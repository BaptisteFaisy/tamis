#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Génère docs/devpost-form.md (anglais, prêt à coller) depuis site/content.json.

Une seule source de vérité : le contenu du site. Ne pas éditer le .md à la main —
relancer `python3 tools/build_devpost.py` après toute modification de content.json.

Usage :
  build_devpost.py                 écrit docs/devpost-form.md
  build_devpost.py --out <fichier> écrit ailleurs (contrôle de dérive en CI)
"""
from __future__ import annotations

import json
import pathlib
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
SITE_URL = "https://tradoooo.github.io/tamis/"
REPO_URL = "https://github.com/Tradoooo/tamis"


def bullets(items: list) -> list:
    return [f"- {i}" for i in items]


def numbered(items: list) -> list:
    return [f"{n}. {i}" for n, i in enumerate(items, 1)]


def section(title: str, body: list) -> list:
    return [f"## {title}", ""] + body + [""]


def head_sections(c: dict, s: dict) -> list:
    L = ["<!-- SPDX-License-Identifier: MIT -->",
         "<!-- GÉNÉRÉ depuis site/content.json par tools/build_devpost.py — ne pas éditer à la main. -->",
         "", "# Tamis — Devpost submission form (ready to paste)", "",
         f"QNX Global Hackathon 2026. Everything below is also live on the demo site: <{SITE_URL}> "
         "(tab **Submission**), and the whole project is public: "
         f"<{REPO_URL}>.", ""]
    L += section("1. Project name", [s["name"]])
    L += section("2. Tagline (one line)", [s["tagline_en"]])
    L += section("3. Category", [s["category_en"]])
    L += section("4. Bonuses claimed", bullets(s["bonuses_en"]))
    L += section("5. The pitch, in one sentence", ["> " + s["pitch_en"]])
    L += section("6. Describe what your project does", s["does_en"])
    L += section("7. Proof of QNX usage (the sensitive point for a host tool)", s["qnx_en"])
    L += section("8. Describe how you built it", s["built_en"])
    return L + section("9. Testing instructions for judges", numbered(s["judges_en"]))


def tail_sections(c: dict) -> list:
    rows = ["| Time | Shot |", "|---|---|"]
    rows += [f"| {v['t']} | {v['en']} |" for v in c["video"]]
    L = section("10. Video script (3 minutes)", rows)
    done = sum(1 for r in c["requirements"] if r["st"] == "done")
    partial = sum(1 for r in c["requirements"] if r["st"] == "partial")
    L += section("11. Requirement coverage", [
        f"{done} of {len(c['requirements'])} spec requirements done, {partial} partial, "
        "the rest blocked on the QNX SDP licence or scheduled next. Full traceability table: "
        f"<{SITE_URL}> tab **Evidence**.", ""])
    L += section("12. Links", bullets([
        f"Demo site (live playground, no install): {SITE_URL}",
        f"Public repository (MIT): {REPO_URL}",
        f"Codelab (20 minutes, runnable): {REPO_URL}/blob/main/docs/codelab/01-auditer-une-politique.md",
        f"Frozen contract: {REPO_URL}/blob/main/CONTRACT.md",
        f"QNX grammar, frozen from the 8.0 SDP and cross-checked with secpolcompile: {REPO_URL}/blob/main/docs/GRAMMAR.md",
        f"100-idea roadmap: {REPO_URL}/blob/main/docs/100-idees.md",
    ]))
    return L + section("13. Checklist", [
        "| # | Field | Status |", "|---|---|---|",
        "| 1 | Public repository, licence visible | **done** — MIT, public |",
        "| 2 | Category selected | **done** — Developer Tooling & Host Tools |",
        "| 3 | Proof of QNX usage written | **done** — section 7 (honest about the pending SDP) |",
        "| 4 | Description of what it does | **done** — section 6 |",
        "| 5 | How it was built | **done** — section 8 |",
        "| 6 | Testing instructions for judges | **done** — section 9 (browser first, then CLI) |",
        "| 7 | Bonuses claimed | **done** — section 4 |",
        "| 8 | Language | **done** — English (site is bilingual FR/EN) |",
        "| 9 | 3-minute video | **to shoot** — script ready, section 10 |",
        "| 10 | Devpost form submitted | **human action** — needs the team's Devpost account |",
    ])


def build(c: dict) -> str:
    s = c["submission"]
    return "\n".join(head_sections(c, s) + tail_sections(c)).rstrip() + "\n"


def main(argv: list) -> int:
    c = json.loads((RACINE / "site" / "content.json").read_text(encoding="utf-8"))
    texte = build(c)
    if "--out" in argv:
        out = pathlib.Path(argv[argv.index("--out") + 1])
    else:
        out = RACINE / "docs" / "devpost-form.md"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(texte, encoding="utf-8")
    print(f"OK {out.relative_to(RACINE) if out.is_relative_to(RACINE) else out} généré")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
