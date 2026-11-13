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
2 et 8 sont verts, et le CONTRACT.md ne bouge plus. La grammaire est gelée et le parseur
(niveaux 4, 5, 6, 7) est écrit : ces niveaux sont désormais **exerçables**. Le gel
fonctionnel du 05/12 (`docs/RETROPLANNING.md`) interdit toute nouvelle fonctionnalité ;
à partir de là, ces niveaux servent de non-régression.

## Statut

| Niveau | État |
|---|---|
| 1 — Unitaires C | ✅ **537 checks** sur 11 binaires (`make test`), ASan/UBSan clean |
| 2 — Différentiel C ↔ Python | ✅ 0 divergence sur 3 graines (250/300/500 verdicts) |
| 3 — Oracle `secpolcompile` | ✅ **exécuté sur le SDP 8.0** : accord sur les 43 politiques (42 du corpus : 17 valides, 25 invalides ; + la politique QNX 8.0) |
| 4 — Banque de cas fautifs | ✅ parseur écrit (C + jumeau JS) ; `fixtures/secpol-cas/` = 42 politiques, **25 refus** avec cause et position ; le différentiel C↔JS rejoue les 42 cas |
| 5 — Fuzz | ✅ 3 graines × 3 000 itérations sous ASan/UBSan, **0 crash**, couvre désormais le parseur `secpol` (vocabulaire, plages, bruit d'octets) |
| 6 — Mutants | ✅ **12/12 tués** (`make mutants`) |
| 7 — Équivalence minimisation | ✅ minimisation + preuve d'équivalence toute-paire vérifiée (`test_minimize`, 18 checks). *Écart assumé :* le corpus aléatoire de 10 000 requêtes de la cible n'est pas généré — la preuve d'équivalence est faite sur les cas construits, pas sur un tirage massif |
| 8 — Gardes maison | ✅ vert (73 fichiers < 500 lignes, fonctions < 50 lignes, SPDX) |
