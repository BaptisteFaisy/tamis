<!-- SPDX-License-Identifier: MIT -->
# Tamis — plan de vérification (TAM-10, cahier §7)

Ce document fige la stratégie de preuve de Tamis. Chaque niveau est exécutable sans
QNX, sans QEMU, sans licence — sauf l'oracle officiel (niveau 3), explicitement optionnel.

## Les 8 niveaux

| # | Niveau | Quoi | Comment | Cible | Sans SDP |
|---|---|---|---|---|---|
| 1 | Unitaires C | chaque module `core/` | un binaire `core/tests/test_<module>.c`, retour 0, `OK <n> checks` | ≥ 800 checks | ✅ |
| 2 | Différentiel C ↔ Python | jumeau indépendant | `tests/test_differentiel.sh`, ≥ 200 verdicts par graine, réduction dichotomique du 1ᵉʳ désaccord | 0 divergence | ✅ |
| 3 | Différentiel oracle | accord avec `secpolcompile` | `oracle/run_oracle.sh` : toute politique acceptée par Tamis est acceptée par le compilateur officiel, et inversement | 0 divergence | ❌ (optionnel, TAM-12) |
| 4 | Banque de cas fautifs | le parseur refuse | 40 politiques invalides (chacune avec l'erreur attendue), 12 chemins dirigés, 8 paires avant/après, 6 jeux de plages cassés | 100 % refusés | ✅ |
| 5 | Fuzz | robustesse | entrées déterministes par graine sous ASan/UBSan | 200 000 entrées, 0 crash/fuite | ✅ |
| 6 | Mutants | les tests mordent | ≥ 10 mutations des moteurs (ordre des règles, sens d'une arête, défaut du refus, borne de plage, compteur de visites) | chacune tuée | ✅ |
| 7 | Équivalence de minimisation | la minimisation préserve la sémantique | 10 000 requêtes de décision rejouées avant/après | 0 divergence | ✅ |
| 8 | Gardes maison | hygiène | `tests/garde.py` (< 500 lignes/fichier, < 50 lignes/fonction), SPDX, `ci-budget` (≤ 5 min/job) | vert | ✅ |

## Commandes

```
make test       # niveaux 1, 2, 4, 7, 8 (C + Python + différentiel + gardes)
make sanitize   # niveau 1 + 5 sous ASan/UBSan
make garde      # niveau 8 seul
oracle/run_oracle.sh --help   # niveau 3 (sort 3 sans SDP, 4 en dry-run)
```

## Règles de la CI (TAM-10, ci-budget)

- `timeout-minutes: 5` par job — **jamais** allongé.
- `concurrency` + `cancel-in-progress: true`.
- `actions/checkout@v4` en `fetch-depth: 1`.
- Jobs parallèles : `core` (unitaires + ASan/UBSan + mutants), `python` (jumeau +
  différentiel + CLI), `fuzz` (campagne courte bornée en temps).
- Job `oracle` **hors PR** : `workflow_dispatch` uniquement, sur un hôte ayant le SDP.
- Aucun build pour cible : Tamis est un outil hôte, sa CI est entièrement portable.

## Critère de gel

Le socle (verdicts + rapports) est **gelé** quand : niveaux 1 (≥ 300 checks sur le socle),
2 et 8 sont verts, et le CONTRACT.md ne bouge plus. Le parseur (niveaux 4, 5, 6, 7) attend
la grammaire gelée (`docs/GRAMMAR.md`, lot L0) — ne pas l'anticiper.

## Statut

| Niveau | État |
|---|---|
| 1 — Unitaires C (socle verdict/rapport) | 🚧 en cours (sous-agent A) |
| 2 — Différentiel C ↔ Python | 🚧 en cours (sous-agent B) |
| 3 — Oracle `secpolcompile` | ⏳ harnais écrit, attend le SDP (optionnel) |
| 4 — Banque de cas fautifs | ⏳ attend le parseur |
| 5 — Fuzz | ⏳ attend le parseur |
| 6 — Mutants | ⏳ attend les moteurs |
| 7 — Équivalence minimisation | ⏳ attend `minimize` |
| 8 — Gardes maison | ✅ vert |
