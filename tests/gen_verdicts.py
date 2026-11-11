#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Générateur déterministe de verdicts pour le différentiel C↔Python (CONTRACT §6).

Deux modes produisent les MÊMES verdicts (même graine) :
  serialize <graine> <n>  -> format TAB/base64 lu par tests/diff_driver.c
  render    <graine> <n>  -> report_json du jumeau Python (host/tamis_report.py)

Le test différentiel compare les deux sorties octet par octet.
"""
from __future__ import annotations

import base64
import os
import random
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "host"))
import tamis_report as T  # noqa: E402

# Alphabet volontairement piégeux : guillemets, backslash, contrôles, UTF-8 multioctet.
ALPHABET = list("abcXY019 \"\\'\n\t\x01\x1féà—→") + ["naïve", "->", "secpol"]
RULE_ALPHABET = list("abR0123456789")


def _rand_str(rng: random.Random, allow_none: bool = True) -> str | None:
    if allow_none and rng.random() < 0.2:
        return None
    n = rng.randint(0, 10)
    return "".join(rng.choice(ALPHABET) for _ in range(n))


def _rand_list(rng: random.Random) -> tuple:
    if rng.random() < 0.45:
        return ()
    n = rng.randint(1, 5)
    return tuple(
        "".join(rng.choice(RULE_ALPHABET) for _ in range(rng.randint(1, 6)))
        for _ in range(n)
    )


def gen(seed: int, n: int) -> list:
    """Génère n verdicts déterministes pour la graine `seed`."""
    rng = random.Random(seed)
    out = []
    for _ in range(n):
        code = rng.randrange(T.VERDICT_COUNT)
        out.append(T.Verdict(
            code=code,
            sev=rng.choice([T.SEV_INFO, T.SEV_WARNING, T.SEV_ERROR]),
            file=_rand_str(rng),
            line=rng.choice([0, rng.randint(1, 9999)]),
            column=rng.choice([0, rng.randint(1, 200)]),
            message=_rand_str(rng, allow_none=False),
            src=_rand_str(rng),
            dst=_rand_str(rng),
            path=_rand_list(rng),
            rules=_rand_list(rng),
        ))
    return out


def _b64(s: str | None) -> str:
    # "@" = sentinelle None (base64("") == "" serait ambigu avec une chaîne vide).
    return "@" if s is None else base64.b64encode(s.encode("utf-8")).decode("ascii")


def _b64list(lst) -> str:
    return ",".join(base64.b64encode(x.encode("utf-8")).decode("ascii") for x in lst)


def serialize(verdicts) -> str:
    """Format TAB/base64 : code,sev,line,column,b64(file),b64(msg),b64(src),b64(dst),path,rules."""
    lines = []
    for v in verdicts:
        lines.append("\t".join([
            str(v.code), str(v.sev), str(v.line), str(v.column),
            _b64(v.file), _b64(v.message), _b64(v.src), _b64(v.dst),
            _b64list(v.path), _b64list(v.rules),
        ]))
    return "\n".join(lines) + ("\n" if lines else "")


def main(argv: list) -> int:
    if len(argv) != 4 or argv[1] not in ("serialize", "render"):
        print("usage : gen_verdicts.py serialize|render <graine> <n>", file=sys.stderr)
        return 2
    seed, n = int(argv[2]), int(argv[3])
    verdicts = gen(seed, n)
    if argv[1] == "serialize":
        sys.stdout.write(serialize(verdicts))
    else:
        sys.stdout.write(T.report_json(verdicts))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
