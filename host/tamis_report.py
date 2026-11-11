#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Tamis — jumeau Python du rendu des verdicts (CONTRACT.md §5, §6).

Implémentation INDÉPENDANTE du cœur C (core/src/rapport.c), pas une copie :
même contrat de sortie, vérifié octet pour octet par tests/test_differentiel.sh.
Python 3 standard uniquement.
"""
from __future__ import annotations

import json
from dataclasses import dataclass, field
from typing import Optional, Sequence

# Noms canoniques des 13 codes, index = tamis_verdict_code (CONTRACT §3).
VERDICT_NAMES = (
    "TAMIS_OK", "TAMIS_PARSE_ERROR", "TAMIS_UNTYPED_PROCESS",
    "TAMIS_MISSING_DEFAULT_SPAWN", "TAMIS_DEAD_RULE", "TAMIS_REDUNDANT_RULE",
    "TAMIS_OVERBROAD_ABILITY", "TAMIS_RANGE_INVALID", "TAMIS_PATH_FOUND",
    "TAMIS_INTERFERENCE_POSSIBLE", "TAMIS_PRIVILEGE_WIDENED",
    "TAMIS_PRIVILEGE_NARROWED", "TAMIS_INCONSISTENT",
)
VERDICT_COUNT = len(VERDICT_NAMES)

# Sévérités (CONTRACT §3).
SEV_INFO, SEV_WARNING, SEV_ERROR = 0, 1, 2
SEV_NAMES = {SEV_INFO: "info", SEV_WARNING: "warning", SEV_ERROR: "error"}
DEFAULT_SEV = (
    SEV_INFO, SEV_ERROR, SEV_WARNING, SEV_WARNING, SEV_INFO, SEV_INFO,
    SEV_WARNING, SEV_ERROR, SEV_ERROR, SEV_ERROR, SEV_ERROR, SEV_INFO, SEV_ERROR,
)
SEV_LABEL = {SEV_ERROR: "ERROR", SEV_WARNING: "WARN", SEV_INFO: "INFO"}


def verdict_name(code: int) -> str:
    """Nom canonique d'un code ; hors plage -> TAMIS_UNKNOWN."""
    return VERDICT_NAMES[code] if 0 <= code < VERDICT_COUNT else "TAMIS_UNKNOWN"


def severity_name(sev: int) -> str:
    """Nom d'une sévérité ; hors plage -> unknown."""
    return SEV_NAMES.get(sev, "unknown")


def default_severity(code: int) -> int:
    """Sévérité par défaut d'un code ; hors plage -> info."""
    return DEFAULT_SEV[code] if 0 <= code < VERDICT_COUNT else SEV_INFO


@dataclass
class Verdict:
    """Miroir de tamis_verdict (core/include/tamis/verdict.h)."""
    code: int = 0
    sev: int = SEV_INFO
    file: Optional[str] = None
    line: int = 0
    column: int = 0
    message: Optional[str] = None
    src: Optional[str] = None
    dst: Optional[str] = None
    path: Sequence[str] = field(default_factory=tuple)
    rules: Sequence[str] = field(default_factory=tuple)


@dataclass
class ReportMeta:
    """Miroir de tamis_report_meta (core/include/tamis/report.h)."""
    policy_file: Optional[str] = None
    policy_sha256: Optional[str] = None
    timestamp_utc: Optional[str] = None
    tamis_version: Optional[str] = None


def _json_obj(v: Verdict) -> str:
    """Un objet JSON de verdict — ordre des clés FIGÉ (CONTRACT §5.2)."""
    o: dict = {"code": verdict_name(v.code), "sev": severity_name(v.sev)}
    if v.src is not None:
        o["src"] = v.src
    if v.dst is not None:
        o["dst"] = v.dst
    if len(v.path) > 0:
        o["hops"] = len(v.path) - 1
        o["path"] = list(v.path)
    if len(v.rules) > 0:
        o["rules"] = list(v.rules)
    if v.file is not None:
        o["file"] = v.file
    if v.line > 0:
        o["line"] = v.line
    if v.column > 0:
        o["column"] = v.column
    o["message"] = v.message if v.message is not None else ""
    return json.dumps(o, separators=(",", ":"), ensure_ascii=False)


def report_json(verdicts: Sequence[Verdict]) -> str:
    """Une ligne par verdict, compact, join par \\n ( trailing \\n inclus)."""
    return "".join(_json_obj(v) + "\n" for v in verdicts)


def _position(v: Verdict) -> str:
    pos = v.file if v.file is not None else "?"
    if v.line > 0:
        pos += ":" + str(v.line)
    if v.column > 0:
        pos += ":" + str(v.column)
    return pos


def _detail(v: Verdict) -> str:
    if len(v.path) > 0:
        src = v.src if v.src is not None else "?"
        dst = v.dst if v.dst is not None else "?"
        d = f"{src} -> {dst} ({len(v.path) - 1} sauts)"
        if len(v.rules) > 0:
            d += " : " + ",".join(v.rules)
        return d
    return v.message if v.message is not None else ""


def report_text(verdicts: Sequence[Verdict]) -> str:
    """Rendu texte (CONTRACT §5.1) : préfixe 7 car., code, position, détail."""
    out = []
    for v in verdicts:
        prefix = SEV_LABEL.get(v.sev, "INFO").ljust(5) + "  "
        out.append(f"{prefix}{verdict_name(v.code)}  {_position(v)}  {_detail(v)}\n")
    errors = sum(1 for v in verdicts if v.sev == SEV_ERROR)
    out.append("OK\n" if errors == 0 else f"FAILED {errors} error(s)\n")
    return "".join(out)


def _md_meta(m: Optional[ReportMeta]) -> str:
    mm = m or ReportMeta()
    return (
        "# Rapport d'audit Tamis\n\n"
        f"- Politique : {mm.policy_file or 'inconnu'}\n"
        f"- Empreinte SHA-256 : {mm.policy_sha256 or 'inconnu'}\n"
        f"- Horodatage (UTC) : {mm.timestamp_utc or 'inconnu'}\n"
        f"- Version de Tamis : {mm.tamis_version or 'inconnu'}\n\n"
    )


def _md_bullets(v: Verdict) -> str:
    s = f"- Position : {_position(v)}\n"
    s += f"- Message : {v.message if v.message is not None else ''}\n"
    if len(v.path) > 0:
        src = v.src if v.src is not None else "?"
        dst = v.dst if v.dst is not None else "?"
        chain = " → ".join(v.path)
        s += f"- Chemin : {src} → {dst} ({len(v.path) - 1} sauts) : {chain}\n"
    if len(v.rules) > 0:
        s += f"- Règles : {', '.join(v.rules)}\n"
    return s


def _md_sections(verdicts: Sequence[Verdict]) -> str:
    s = ""
    for code in range(VERDICT_COUNT):
        vs = [v for v in verdicts if v.code == code]
        if not vs:
            continue
        s += f"### {verdict_name(code)} — {severity_name(default_severity(code))}"
        s += f" (×{len(vs)})\n"
        for v in vs:
            s += _md_bullets(v)
        s += "\n"
    return s


def report_markdown(verdicts: Sequence[Verdict], meta: Optional[ReportMeta] = None) -> str:
    """Rendu Markdown d'audit (CONTRACT §5.3)."""
    err = sum(1 for v in verdicts if v.sev == SEV_ERROR)
    warn = sum(1 for v in verdicts if v.sev == SEV_WARNING)
    info = sum(1 for v in verdicts if v.sev == SEV_INFO)
    s = _md_meta(meta)
    s += "## Synthèse\n\n| Sévérité | Nombre |\n|---|---|\n"
    s += f"| error | {err} |\n| warning | {warn} |\n| info | {info} |\n\n"
    s += "## Verdicts\n\n"
    s += _md_sections(verdicts)
    s += "## Propriétés vérifiées\n\n"
    s += "(une section par propriété : énoncé, méthode, corpus, résultat, contre-exemple)\n"
    return s


def exit_code(verdicts: Sequence[Verdict]) -> int:
    """1 si au moins un verdict de sévérité error, 0 sinon (CI)."""
    return 1 if any(v.sev == SEV_ERROR for v in verdicts) else 0
