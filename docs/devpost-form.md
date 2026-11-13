<!-- SPDX-License-Identifier: MIT -->
<!-- GÉNÉRÉ depuis site/content.json par tools/build_devpost.py — ne pas éditer à la main. -->

# Tamis — Devpost submission form (ready to paste)

QNX Global Hackathon 2026. Everything below is also live on the demo site: <https://tradoooo.github.io/tamis/> (tab **Submission**), and the whole project is public: <https://github.com/Tradoooo/tamis>.

## 1. Project name

Tamis

## 2. Tagline (one line)

Static analysis and proof for QNX 8.0 security policies — before deployment.

## 3. Category

Developer Tooling & Host Tools

## 4. Bonuses claimed

- Best use of QNX-specific Technologies (security policies are explicitly named in the rules)
- Best Codelab/Tutorial (self-contained 20-minute codelab)

## 5. The pitch, in one sentence

> QNX generates and monitors security policies; QNX asks the integrator to review, merge and justify them by hand. Tamis tools exactly that part — and proves itself by differential testing against secpolcompile.

## 6. Describe what your project does

Tamis is a host tool that reads a QNX security policy before deployment and answers four questions the official toolchain never asks: is this rule reachable? can this type reach that critical type through a chain of intermediaries? which rules are dead? does this change widen a privilege?
The promise: a security policy stops being text you re-read by hand and becomes an artifact that is analyzed, minimized, diffable and proved — with a non-interference certificate usable in an ISO 26262, IEC 62304, EN 50128 or UNECE R155 file.
Tamis never duplicates the official chain. QNX generates (secpolgenerate, with incremental diff), compiles (secpolcompile), applies (secpolpush) and monitors (secpolmonitor). QNX's own documentation states that everything downstream is manual. Tamis fills that space, and only that space.

## 7. Proof of QNX usage (the sensitive point for a host tool)

The language modelled is QNX's security policy language: type, attribute, class, range, default_rules, allow, allow_attach, allow_link, default_spawn_type, derive_type, mount_point, the nonroot/unlock/noinherit/gain_priv modifiers and the * and ... wildcards — each recorded with its source page (secpol_language.html, using_security_policy_types.html, named_range_file.html).
politique/abilities.tsv inventories 75 QNX abilities (static procmgr ones and dynamic custom ones: fsevmgr/, iofunc/, io-gpio/, network/, smmu/, vfs/) with their scope and source (abilities_reference.html).
The 13 verdict codes derive directly from the 15 gaps QNX's own documentation admits: manual review of unused rules, generated policy not usable as is, no static analysis before deployment, default_spawn_type never generated, privilege elevation by trial and error, manual merging, manual typing verification, named ranges not validatable cold, typed objects limited to channels.
oracle/run_oracle.sh runs secpolcompile — a host utility of the SDP — and compares its accept/reject verdict with ours, policy by policy. That is the differential against the official QNX compiler (TAM-12).
EVIDENCE FROM A REAL SYSTEM: with the QNX 8.0 docs returning 404, a real QNX Neutrino 7.1.0 was booted under QEMU and queried on 2026-11-12. Artefacts are published in preuve/qnx71/ (raw session, SHA-256 fingerprint of the image, reproduction procedure). Result: 66 ability names extracted from the binary, the «;» statement terminator, the exact forms of derive_type / allow_link / allow_attach, the 8 modifier strings of the 7.1 binary (allow, deny, inherit, no-inherit, nonroot, root, lock, subrange) — not to be confused with the 8.0 source syntax, where the compiler accepts only nonroot, unlock, noinherit, the /proc/boot/secpol.bin path, the compiled blob structure (SECPOL_BLOB_*) and the official integrity checks. Lesson learned: a binary's internal strings are not the source syntax — the 8.0 compiler is the authority. The QNX image is not redistributed — only its fingerprint is published.
Real status as of 2026-11-13: the QNX 8.0 SDP is installed on the development machine. The grammar is frozen (76 productions extracted from the official docs, cross-checked with the compiler), and the real secpol parser is written in C and in JavaScript — both produce the same model and reject the same inputs with the same line and column. The differential against secpolcompile runs: full agreement on 43 policies (42-policy corpus — 17 valid accepted, 25 invalid rejected — plus the QNX 8.0 policy). The strict parser refuses what it cannot read — no approximate analysis.

## 8. Describe how you built it

core/ is pure C11: no clock, no file, no socket, no global state, no allocation — the caller supplies an arena. That is what makes the kernel unit-testable and embeddable in WASM later.
Three independent implementations, one contract: the C core (authoritative), a Python twin and a JavaScript twin (the one running this site). All three render byte-identical JSON, continuously checked by tests/test_differentiel.sh (C↔Python, 250/300/500 random verdicts) and tests/test_js.sh (C↔JS, 13 real scenarios).
Frozen pipeline: text → lex → ast → model(graph) → {reach, proof, minimize, diff, lint, explain} → verdicts → report. No stage feeds back; any error yields TAMIS_PARSE_ERROR with position and cause, never an approximate analysis.
Quality: 9,876 green checks, 9,000 fuzz inputs under ASan/UBSan with no crash, 12 mutants all killed, a size guard (< 500 lines per file, < 50 per function), and a full CI in 24 s against a 5-minute budget.

## 9. Testing instructions for judges

1. Open this site: no account, no install, no download. Everything runs in the browser.
2. Playground tab: the «attaque» policy is loaded by default. Click lint — two untyped processes and the missing default_spawn_type show up.
3. Click reach with src=app_tiers, dst=pompe_critique — the 3-hop attack path appears, highlighted red in the graph, with rules R112, R87, R9.
4. Click explain — the guilty chain in plain words and the minimal fix (remove R9) are given.
5. Load the «durcie» (hardened) example then click proof — the non-interference certificate turns green, with the number of nodes visited: absence is proved, not assumed.
6. Click minimize then diff — redundant rule R999 is removed with an equivalence proof, and the diff shows only a privilege narrowing, no widening.
7. Check that the browser is not smoke and mirrors: clone the repo, make test (9,876 checks), make mutants (12 killed), make sanitize (ASan/UBSan). The build/tamis CLI reproduces exactly what the site displays.

## 10. Video script (3 minutes)

| Time | Shot |
|---|---|
| 0:00–0:20 | QNX's own docs: «the generated policy is not usable as is», «reviewing unused rules is manual», «no analysis before deployment». Three quotes, three gaps. |
| 0:20–0:50 | tamis lint policy.secpol on 400 rules: 88 dead rules, 3 over-broad abilities, 2 untyped processes. |
| 0:50–1:40 | tamis reach --src app_tiers --dst pompe_critique: a 3-hop path, rules cited, line in the file. Cut the guilty rule, re-run: no path, green non-interference certificate. |
| 1:40–2:10 | tamis minimize: 400 → 212 rules, plus the equivalence proof (10,000 identical decisions, zero divergence). |
| 2:10–2:40 | tamis diff v1.secpol v2.secpol in a PR: a privilege widening flagged, CI turns red, automatic comment. |
| 2:40–3:00 | The evidence: check count, mutants killed, fuzz, the C ↔ Python ↔ JS differential, and (when available) agreement with secpolcompile. |

## 11. Requirement coverage

16 of 24 spec requirements done, 0 partial, the rest blocked on the QNX SDP licence or scheduled next. Full traceability table: <https://tradoooo.github.io/tamis/> tab **Evidence**.


## 12. Links

- Demo site (live playground, no install): https://tradoooo.github.io/tamis/
- Public repository (MIT): https://github.com/Tradoooo/tamis
- Codelab (20 minutes, runnable): https://github.com/Tradoooo/tamis/blob/main/docs/codelab/01-auditer-une-politique.md
- Frozen contract: https://github.com/Tradoooo/tamis/blob/main/CONTRACT.md
- Recorded QNX grammar (sourced, with TO VERIFY markers): https://github.com/Tradoooo/tamis/blob/main/docs/GRAMMAR.md
- 100-idea roadmap: https://github.com/Tradoooo/tamis/blob/main/docs/100-idees.md

## 13. Checklist

| # | Field | Status |
|---|---|---|
| 1 | Public repository, licence visible | **done** — MIT, public |
| 2 | Category selected | **done** — Developer Tooling & Host Tools |
| 3 | Proof of QNX usage written | **done** — section 7 (honest about the pending SDP) |
| 4 | Description of what it does | **done** — section 6 |
| 5 | How it was built | **done** — section 8 |
| 6 | Testing instructions for judges | **done** — section 9 (browser first, then CLI) |
| 7 | Bonuses claimed | **done** — section 4 |
| 8 | Language | **done** — English (site is bilingual FR/EN) |
| 9 | 3-minute video | **to shoot** — script ready, section 10 |
| 10 | Devpost form submitted | **human action** — needs the team's Devpost account |
