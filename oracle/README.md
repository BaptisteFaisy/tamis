<!-- SPDX-License-Identifier: MIT -->
# oracle/ — différentiel contre `secpolcompile` (TAM-12)

**Le point fort décisif de Tamis** : `secpolcompile` est un utilitaire **hôte** (il tourne
sur le poste de développement, pas sur la cible). Tamis peut donc être testé **par
différentiel contre le compilateur officiel sans aucune cible QNX** — c'est la preuve la
plus forte disponible, et elle ne demande ni QEMU ni cible matérielle.

## Principe

Pour chaque politique du corpus :
- `secpolcompile` l'**accepte** ou la **refuse** (avec une erreur).
- Tamis l'**accepte** ou la **refuse** (verdict `TAMIS_PARSE_ERROR` + position + cause).
- Les deux **doivent être d'accord**. Un désaccord est un bug de Tamis (ou un recoin de
  grammaire non couvert — dans ce cas Tamis doit refuser et le dire, jamais approximer).

## État — optionnel, ne bloque rien

Le SDP QNX 8.0 n'est **pas installé** sur le poste (licence gratuite à demander, action
humaine). Sans `secpolcompile`, `run_oracle.sh` sort **3** (prérequis manquant) — ce n'est
pas un échec : le différentiel **C ↔ Python** (`tests/test_differentiel.sh`, niveau 2 du
plan de vérification) reste la preuve principale et tourne sans SDP.

Ce harnais est donc écrit **maintenant** (pour être prêt le jour où le SDP arrive) mais
n'est pas exécuté en CI de PR : il est **hors PR**, sur `workflow_dispatch`, uniquement sur
un hôte ayant le SDP (cahier §8, règle ci-budget).

## Usage

```
oracle/run_oracle.sh --help          # aide
oracle/run_oracle.sh --dry-run       # sort 4 (aucun résultat possible)
oracle/run_oracle.sh                 # sort 3 sans SDP ; sinon différentiel réel
oracle/run_oracle.sh --fixtures fixtures/
```

## À brancher (lot L1)

La ligne `tamis="accepte"` est un **TODO** : elle sera remplacée par l'appel réel au
parseur Tamis (`core/src/lex.c` + `ast.c`) dès que la grammaire sera gelée
(`docs/GRAMMAR.md`, lot L0) et le parseur écrit. Ne pas anticiper.
