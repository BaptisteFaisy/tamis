#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Tests de mutation (validation niveau 6) : chaque mutant DOIT être tué par la suite.

Applique une mutation à une source du core, reconstruit le test ciblé, vérifie qu'il ÉCHOUE,
puis restaure la source. Un mutant qui survit = un trou dans les tests -> échec global.
Lancer : `make mutants` (ou `python3 tests/mutants.py`).
"""
import glob
import os
import subprocess
import sys

RACINE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CFLAGS = ["-std=c11", "-O1", "-w", "-Icore/include"]
CORE = sorted(glob.glob(os.path.join(RACINE, "core", "src", "*.c")))

# (nom, fichier, motif_original, mutation, test_cible)
MUTANTS = [
    ("verdict-nom", "core/src/verdict.c", '"TAMIS_PATH_FOUND",', '"TAMIS_PATH_XXX",', "test_verdict"),
    ("verdict-sev-defaut", "core/src/verdict.c",
     "TAMIS_SEV_ERROR,   /* TAMIS_PATH_FOUND */", "TAMIS_SEV_INFO,   /* TAMIS_PATH_FOUND */", "test_verdict"),
    ("rapport-texte-fleche", "core/src/rapport.c", 'w_puts(w, " -> ");', 'w_puts(w, " => ");', "test_rapport"),
    ("rapport-json-line", "core/src/rapport.c",
     'if (v->line > 0) { jkey(w, &first, "line")', 'if (v->line >= 0) { jkey(w, &first, "line")', "test_rapport"),
    ("model-compte-types", "core/src/model.c", "c->types++;", "c->types += 2;", "test_model"),
    ("reach-sens-arete", "core/src/reach.c",
     "if (m->rules[r].src != u) {", "if (m->rules[r].dst != u) {", "test_reach"),
    ("reach-compte-visites", "core/src/reach.c", "nvis = 1;", "nvis = 2;", "test_reach"),
    ("proof-code-certificat", "core/src/proof.c", "v.code = TAMIS_OK;", "v.code = TAMIS_INCONSISTENT;", "test_proof"),
    ("minimize-glouton", "core/src/minimize.c",
     "red = reach_in(&minm, m->rules[i].src, m->rules[i].dst, o->arena);", "red = 0;", "test_minimize"),
    ("diff-sens-elargissement", "core/src/diff.c",
     "if (emit_edge_change(o, cand, r, 1) != 0)", "if (emit_edge_change(o, cand, r, 0) != 0)", "test_diff"),
    ("lint-severite", "core/src/lint.c", "v.sev = TAMIS_SEV_WARNING;", "v.sev = TAMIS_SEV_ERROR;", "test_lint"),
    ("explain-correctif", "core/src/explain.c", "Correctif minimal", "Correctif XXX", "test_explain"),
]


def build_run(test, binpath):
    src = os.path.join(RACINE, "core", "tests", test + ".c")
    cmd = ["gcc"] + CFLAGS + ["-o", binpath, src] + CORE
    if subprocess.run(cmd, cwd=RACINE, capture_output=True).returncode != 0:
        return None  # mutant non compilable (stillborn)
    try:
        return subprocess.run([binpath], cwd=RACINE, capture_output=True, timeout=120).returncode
    except subprocess.TimeoutExpired:
        return 99  # boucle infinie introduite = tué


def main():
    killed = survived = stillborn = missing = 0
    for i, (nom, frel, old, new, test) in enumerate(MUTANTS):
        path = os.path.join(RACINE, frel)
        with open(path, encoding="utf-8") as fh:
            orig = fh.read()
        if old not in orig:
            print(f"KO   {nom} : motif absent de {frel}")
            missing += 1
            continue
        binpath = f"/tmp/tamis_mut_{os.getpid()}_{i}"
        try:
            with open(path, "w", encoding="utf-8") as fh:
                fh.write(orig.replace(old, new, 1))
            rc = build_run(test, binpath)
            if rc is None:
                print(f"INFO {nom} : mutant non compilable (stillborn, exclu)")
                stillborn += 1
            elif rc != 0:
                print(f"OK   {nom} : tué par {test}")
                killed += 1
            else:
                print(f"KO   {nom} : SURVÉCU à {test} — trou dans les tests")
                survived += 1
        finally:
            with open(path, "w", encoding="utf-8") as fh:
                fh.write(orig)
            if os.path.exists(binpath):
                os.remove(binpath)
    if survived or missing:
        print(f"FAILED {survived} survivant(s) + {missing} motif(s) absent(s)")
        return 1
    print(f"OK {killed} checks (mutants tués ; {stillborn} stillborn exclu(s))")
    return 0


if __name__ == "__main__":
    sys.exit(main())
