#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Tamis — tests du jumeau Python (CONTRACT.md §5, §6). Binaire autonome, OK <n> checks."""
from __future__ import annotations

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import tamis_report as T  # noqa: E402

_CHECKS = 0
_FAIL = 0


def check(cond: bool, msg: str) -> None:
    global _CHECKS, _FAIL
    _CHECKS += 1
    if not cond:
        print(f"FAILED : {msg}")
        _FAIL = 1


def eq(got: str, want: str, msg: str) -> None:
    global _CHECKS, _FAIL
    _CHECKS += 1
    if got != want:
        print(f"FAILED : {msg}\n  attendu : [{want}]\n  obtenu  : [{got}]")
        _FAIL = 1


def make_path() -> T.Verdict:
    return T.Verdict(
        code=8, sev=T.SEV_ERROR, src="app_tiers", dst="pompe_critique",
        path=("app_tiers", "journal", "diag", "pompe_critique"),
        rules=("R112", "R87", "R9"), file="policy.secpol", line=214,
        message="chemin de privilège atteignable",
    )


def test_names() -> None:
    for i in range(T.VERDICT_COUNT):
        check(T.verdict_name(i).startswith("TAMIS_"), f"nom {i} préfixe")
        check(T.verdict_name(i) == T.VERDICT_NAMES[i], f"nom {i} exact")
    eq(T.verdict_name(T.VERDICT_COUNT), "TAMIS_UNKNOWN", "hors plage -> UNKNOWN")
    eq(T.verdict_name(-1), "TAMIS_UNKNOWN", "négatif -> UNKNOWN")
    eq(T.severity_name(T.SEV_INFO), "info", "info")
    eq(T.severity_name(T.SEV_WARNING), "warning", "warning")
    eq(T.severity_name(T.SEV_ERROR), "error", "error")
    eq(T.severity_name(99), "unknown", "sev inconnue")
    eq(T.DEFAULT_SEV[1], T.SEV_ERROR, "PARSE_ERROR -> error")
    eq(T.DEFAULT_SEV[4], T.SEV_INFO, "DEAD_RULE -> info")
    check(len(T.DEFAULT_SEV) == T.VERDICT_COUNT, "13 sévérités par défaut")


def test_json_canonical() -> None:
    want = (
        '{"code":"TAMIS_PATH_FOUND","sev":"error","src":"app_tiers",'
        '"dst":"pompe_critique","hops":3,'
        '"path":["app_tiers","journal","diag","pompe_critique"],'
        '"rules":["R112","R87","R9"],"file":"policy.secpol","line":214,'
        '"message":"chemin de privilège atteignable"}\n'
    )
    eq(T.report_json([make_path()]), want, "JSON canonique octet pour octet")


def test_json_omission() -> None:
    v = T.Verdict(code=0, sev=T.SEV_INFO, message="rien")
    eq(T.report_json([v]), '{"code":"TAMIS_OK","sev":"info","message":"rien"}\n',
       "clés optionnelles omises")
    out = T.report_json([v])
    for k in ("src", "dst", "hops", "path", "rules", "file", "line", "column"):
        check(f'"{k}"' not in out, f"pas de clé {k}")


def test_json_column() -> None:
    v = T.Verdict(code=1, sev=T.SEV_ERROR, file="p.secpol", line=7, column=12,
                  message="jeton inattendu")
    eq(T.report_json([v]),
       '{"code":"TAMIS_PARSE_ERROR","sev":"error","file":"p.secpol",'
       '"line":7,"column":12,"message":"jeton inattendu"}\n',
       "position file:line:column")


def test_json_escaping() -> None:
    v = T.Verdict(code=0, sev=T.SEV_INFO, message='a"b\\c\nd\te\bf\fg\rh')
    out = T.report_json([v])
    check('\\"' in out, "guillemet échappé")
    check("\\\\" in out, "backslash échappé")
    for esc in ("\\n", "\\t", "\\b", "\\f", "\\r"):
        check(esc in out, f"{esc} présent")
    check('"a\\"b\\\\c\\nd\\te\\bf\\fg\\rh"' in out, "chaîne complète échappée")
    c = T.Verdict(code=0, sev=T.SEV_INFO, message="x\x01y\x1fz")
    oc = T.report_json([c])
    check("\\u0001" in oc, "contrôle 0x01 -> \\u0001")
    check("\\u001f" in oc, "contrôle 0x1f -> \\u001f")
    u = T.Verdict(code=0, sev=T.SEV_INFO, message="sécurité—naïve")
    ou = T.report_json([u])
    check("sécurité—naïve" in ou, "UTF-8 non échappé")
    check("\\u00e9" not in ou, "pas de \\u pour é")


def test_text() -> None:
    out = T.report_text([make_path()])
    check("ERROR  TAMIS_PATH_FOUND  policy.secpol:214  "
          "app_tiers -> pompe_critique (3 sauts) : R112,R87,R9\n" in out,
          "ligne de chemin exacte")
    check("FAILED 1 error(s)\n" in out, "synthèse FAILED")
    w = T.Verdict(code=2, sev=T.SEV_WARNING, file="policy.secpol", line=88,
                  message="processus non typé")
    ow = T.report_text([w])
    check("WARN   TAMIS_UNTYPED_PROCESS  policy.secpol:88  processus non typé\n" in ow,
          "ligne warning exacte")
    check("OK\n" in ow, "synthèse OK")
    d = T.Verdict(code=4, sev=T.SEV_INFO, file="policy.secpol", line=120,
                  message="règle jamais atteinte : R42")
    od = T.report_text([d])
    check("INFO   TAMIS_DEAD_RULE  policy.secpol:120  règle jamais atteinte : R42\n" in od,
          "ligne info exacte")
    # alignement : préfixe de 7 caractères
    for line in (out + ow + od).splitlines():
        if line.startswith(("ERROR", "WARN", "INFO")):
            check(line[5:7] == "  " or line[4:7] == "   ", f"alignement préfixe : {line[:7]}")


def test_markdown() -> None:
    m = T.ReportMeta("policy.secpol", "ab" * 32, "2026-11-11T22:00:00Z", "0.1.0")
    out = T.report_markdown([make_path()], m)
    check("# Rapport d'audit Tamis\n" in out, "titre")
    check("- Politique : policy.secpol\n" in out, "politique")
    check("- Empreinte SHA-256 : " + "ab" * 32 in out, "empreinte")
    check("- Horodatage (UTC) : 2026-11-11T22:00:00Z\n" in out, "horodatage")
    check("- Version de Tamis : 0.1.0\n" in out, "version")
    check("## Synthèse\n" in out, "synthèse")
    check("| error | 1 |\n" in out, "compteur error")
    check("### TAMIS_PATH_FOUND — error (×1)\n" in out, "section code")
    check("- Position : policy.secpol:214\n" in out, "position")
    check("- Chemin : app_tiers → pompe_critique (3 sauts) : "
          "app_tiers → journal → diag → pompe_critique\n" in out, "chemin")
    check("- Règles : R112, R87, R9\n" in out, "règles")
    check("## Propriétés vérifiées\n" in out, "propriétés")
    on = T.report_markdown([T.Verdict(code=0, sev=T.SEV_INFO, message="ok")], None)
    check("- Politique : inconnu\n" in on, "meta None -> inconnu")
    check("- Version de Tamis : inconnu\n" in on, "meta None -> version inconnu")


def test_exit_and_multi() -> None:
    vs = [make_path(),
          T.Verdict(code=2, sev=T.SEV_WARNING, message="x"),
          T.Verdict(code=4, sev=T.SEV_INFO, message="y")]
    check(T.exit_code(vs) == 1, "exit_code 1 si error")
    check(T.exit_code(vs[1:]) == 0, "exit_code 0 sans error")
    check(T.exit_code([]) == 0, "exit_code vide -> 0")
    j = T.report_json(vs)
    check(j.count("\n") == 3, "3 lignes JSON")
    check(j.endswith("\n"), "JSON termine par \\n")
    eq(T.report_json([]), "", "JSON vide")


def main() -> int:
    test_names()
    test_json_canonical()
    test_json_omission()
    test_json_column()
    test_json_escaping()
    test_text()
    test_markdown()
    test_exit_and_multi()
    if _FAIL:
        print("FAILED test_tamis_report")
        return 1
    print(f"OK {_CHECKS} checks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
