<!-- SPDX-License-Identifier: MIT -->
# Tamis — décisions de conception (ADR)

Format court : une décision = contexte → décision → conséquence. Les plus récentes en haut.

---

## D-005 — Différentiel C↔Python par sérialisation base64 (2026-11-11)

**Contexte.** Le jumeau Python (`host/tamis_report.py`) et le cœur C (`core/src/rapport.c`)
doivent produire des sorties **octet pour octet** identiques (TAM-09). Il faut donc leur
soumettre exactement les mêmes verdicts, y compris des chaînes piégeuses (guillemets,
backslash, contrôles, UTF-8 multioctet).

**Décision.** `tests/gen_verdicts.py` génère des verdicts déterministes par graine et les
émet dans deux modes partageant la même génération : `serialize` (format TAB + base64 lu par
`tests/diff_driver.c`) et `render` (report_json Python). Le base64 évite toute ambiguïté de
délimiteur ; la sentinelle `@` distingue `None` (champ absent) de la chaîne vide `""`.
`tests/test_differentiel.sh` compile le pont C, compare octet par octet et localise le
premier désaccord.

**Conséquence.** 0 divergence vérifiée sur 3 graines (250/300/500 verdicts). Ajouter un
moteur au différentiel = étendre `gen_verdicts.py`, pas réécrire un harnais.

## D-004 — Mode sous-ensemble strict (2026-11-11)

**Contexte.** La grammaire exacte du langage `secpol` n'est pas encore gelée
(`docs/GRAMMAR.md` est relevé depuis l'inventaire documenté, doc officielle inaccessible).
Risque : inventer une syntaxe et produire une analyse faussement rassurante.

**Décision.** Tamis applique le **mode sous-ensemble strict** : il refuse d'analyser ce qu'il
ne sait pas parser et le dit (`TAMIS_PARSE_ERROR` avec position et cause). **Jamais**
d'analyse approximative d'une politique partiellement comprise (CONTRACT §1).

**Conséquence.** Le parseur (`core/src/lex.c`, `ast.c`) **n'est pas écrit** tant que
`docs/GRAMMAR.md` n'est pas gelé. Le socle (verdicts + rapports) est indépendant de la
grammaire et reste livrable et testable dès maintenant.

## D-003 — Socle C↔Python indépendant de la grammaire (2026-11-11)

**Contexte.** Tout le projet était à l'arrêt (0 ligne) sur le lot L0 (grammaire), bloquant.

**Décision.** Découpler : livrer d'abord le **socle verdicts + rapports** (13 codes, 3 rendus
texte/JSON/Markdown d'audit, jumeau Python, différentiel), qui ne dépend d'aucune syntaxe.
La grammaire et le parseur viennent ensuite, par-dessus.

**Conséquence.** 623 vérifications vertes (287 C + 86 Python + 250 différentiel) dès le
premier jalon, sans SDP ni cible. Le `CONTRACT.md` est gelé et sert de référence aux deux
implémentations.

## D-002 — Cœur pur, métadonnées fournies par l'appelant (2026-11-11)

**Contexte.** Le rapport d'audit a besoin d'une empreinte SHA-256, d'un horodatage et d'une
version — autant d'éléments qui tentent vers l'E/S et l'horloge, interdits dans `core/`.

**Décision.** `core/` est **pur** : ni horloge, ni fichier, ni socket, ni état global mutable,
aucune allocation (l'appelant fournit les tampons). Les métadonnées passent par la structure
`tamis_report_meta` remplie par l'appelant (`host/`).

**Conséquence.** Le cœur est testable unitairement et embarquable (plus tard en WASM). Le
calcul SHA-256 et l'horodatage vivent dans `host/`, jamais dans `core/`.

## D-001 — Dépôt standalone `Tradoooo/tamis` (2026-11-11)

**Contexte.** Le socle partagé vit dans `qnx-kit/tamis/` (lot F, sans remote). La soumission
au QNX Global Hackathon exige un **dépôt public** dédié (cahier §13).

**Décision.** Dépôt public standalone `Tradoooo/tamis` (MIT), suivant l'architecture du
cahier §4. `GaiaSG94` invité **contributor** (permission write). Le lot F de `qnx-kit` a
vocation à être replié dans ce dépôt.

**Conséquence.** CI GitHub dédiée (`ci.yml` ≤ 5 min/job, `oracle.yml` hors PR sur
`workflow_dispatch`). Aucun chemin absolu codé en dur (garde CI).

---

## Blocages ouverts (actions humaines)

| Blocage | Qui | Effort |
|---|---|---|
| Licence QNX SDP 8.0 non demandée | **Baptiste** — compte myQNX, formulaire QNX Everywhere | 10 min + ~1 h d'attente |
| Doc officielle en ligne inaccessible (404) | — | contourner via le SDP installé, ou accès hors-ligne |

Tant que ces deux points ne sont pas levés : `docs/GRAMMAR.md` reste `À VÉRIFIER`, le parseur
n'est pas écrit, et le différentiel contre `secpolcompile` (`oracle/`, TAM-12) sort 3
(prérequis manquant) — sans bloquer le reste.
