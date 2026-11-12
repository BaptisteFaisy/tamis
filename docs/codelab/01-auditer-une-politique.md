<!-- SPDX-License-Identifier: MIT -->
# Codelab 1 — Auditer une politique de sécurité QNX en 20 minutes

> Objectif : partir d'une politique **trouée**, trouver le chemin d'attaque, le **prouver**,
> le corriger, puis **prouver** la non-interférence — le tout sans QNX, sans QEMU, sans licence.
> Bonus visé : *Best Codelab/Tutorial* (QNX Global Hackathon 2026).

## Prérequis

- `gcc` (C11), `python3` (≥ 3.8, stdlib seule), `make`. Aucune dépendance à installer.
- Le dépôt : `git clone https://github.com/Tradoooo/tamis && cd tamis`.

> ⚠️ Les politiques de ce codelab sont au **format provisoire** (`type`/`allow`/`untyped`,
> voir `core/src/model.c`). Le parseur du vrai langage `secpol` arrive dès que la grammaire
> est gelée (`docs/GRAMMAR.md`) ; les moteurs, eux, tournent déjà sur le même modèle.

## Étape 0 — Construire (1 min)

```
make cli
```
Construit `build/tamis`. Vérification :
```
build/tamis version
# tamis 0.1.0
```

## Étape 1 — La politique vulnérable (2 min)

`fixtures/attaque.mpf` modélise un système où un tiers applicatif (`app_tiers`) peut,
de proche en proche, atteindre une `pompe_critique` :

```
type app_tiers
type journal
type diag
type pompe_critique
type reseau
allow app_tiers journal msg @R112
allow journal diag msg @R87
allow diag pompe_critique control @R9
allow reseau journal msg @R10
allow app_tiers journal msg @R999     # doublon de R112
untyped /bin/legacy
untyped /bin/foo
```

## Étape 2 — Lint : le moindre privilège d'abord (2 min)

```
build/tamis lint fixtures/attaque.mpf
```
```
WARN   TAMIS_UNTYPED_PROCESS  fixtures/attaque.mpf:21  processus non typé : /bin/legacy
WARN   TAMIS_UNTYPED_PROCESS  fixtures/attaque.mpf:22  processus non typé : /bin/foo
WARN   TAMIS_MISSING_DEFAULT_SPAWN  fixtures/attaque.mpf  default_spawn_type absent : …
OK
```
**Lecture** : deux processus sans type (ils héritent d'un type par défaut non maîtrisé —
trou documenté S10) et aucun `default_spawn_type` (trou S7 : jamais généré par QNX).

## Étape 3 — Reachability : trouver le chemin d'attaque (2 min)

```
build/tamis reach fixtures/attaque.mpf --src app_tiers --dst pompe_critique
```
```
ERROR  TAMIS_PATH_FOUND  fixtures/attaque.mpf  app_tiers -> pompe_critique (3 sauts) : R112,R87,R9
FAILED 1 error(s)
```
**Lecture** : `app_tiers` atteint `pompe_critique` en 3 sauts, via les règles R112, R87, R9.
Code de retour **1** (erreur) — utilisable tel quel comme garde de CI.

## Étape 4 — Explain : pourquoi, et le correctif minimal (3 min)

```
build/tamis explain fixtures/attaque.mpf --src app_tiers --dst pompe_critique
```
```
ERROR  TAMIS_PATH_FOUND  …  app_tiers -> pompe_critique (3 sauts) : R112,R87,R9
INFO   TAMIS_OK  …  Pourquoi ce refus : app_tiers atteint pompe_critique via la chaîne
       app_tiers →(R112) journal →(R87) diag →(R9) pompe_critique. Correctif minimal :
       retirer la règle R9 (dernière arête avant pompe_critique) tue ce chemin ; relancer
       `tamis proof …` pour obtenir le certificat de non-interférence.
```
C'est la fonction qui rend l'outil utilisable par quelqu'un d'autre que son auteur :
la chaîne coupable **en clair** et le correctif **minimal** proposé.

## Étape 5 — Corriger et PREUVER la non-interférence (3 min)

`fixtures/durcie.mpf` est la version corrigée (R9 et le doublon R999 supprimés,
`default_spawn` ajouté). Prouvons que le chemin est mort :
```
build/tamis proof fixtures/durcie.mpf --src app_tiers --dst pompe_critique
```
```
INFO   TAMIS_OK  fixtures/durcie.mpf  non-interférence prouvée : app_tiers n'atteint pas
       pompe_critique — 3 nœud(s) visité(s), parcours exhaustif
OK
```
**La différence avec un linter** : l'absence de chemin est **prouvée** par un parcours
**exhaustif** (le compteur de visites est publié), pas seulement « non trouvée ». Code de
retour **0**.

## Étape 6 — Minimize : retirer les règles mortes, à preuve (2 min)

```
build/tamis minimize fixtures/attaque.mpf
```
```
INFO   TAMIS_REDUNDANT_RULE  fixtures/attaque.mpf:18  règle redondante R999 : app_tiers atteint déjà journal sans elle
INFO   TAMIS_OK  fixtures/attaque.mpf  minimisation équivalente : 1 règle(s) retirée(s), 25 paire(s) d'accessibilité identiques
OK
```
R999 est un doublon de R112. La **preuve d'équivalence** rejoue l'accessibilité toute-paire
avant/après : 25 paires identiques → la minimisation ne change aucun comportement.

## Étape 7 — Diff : la non-régression de privilège (2 min)

```
build/tamis diff fixtures/attaque.mpf fixtures/durcie.mpf
```
```
INFO   TAMIS_PRIVILEGE_NARROWED  fixtures/attaque.mpf  réduction de privilège : diag -> pompe_critique [control]
OK
```
Passer de la politique trouée à la durcie **réduit** un privilège (R9 retirée) et n'en
**élargit** aucun. Dans une CI, tout `TAMIS_PRIVILEGE_WIDENED` ferait passer le diff au rouge.

## Étape 8 — View : visualiser le graphe (2 min)

```
build/tamis view fixtures/attaque.mpf --src app_tiers --dst pompe_critique > /tmp/graphe.json
```
Ouvre `view/index.html` dans un navigateur, colle le JSON : le graphe de privilèges s'affiche,
le **chemin d'attaque en rouge**. Zéro installation, hors-ligne.

## Étape 9 — Le rapport d'audit (1 min)

```
build/tamis lint fixtures/attaque.mpf --format md
```
Produit un rapport Markdown d'audit (synthèse, verdicts, **empreinte SHA-256** de la
politique, horodatage, version) — versable tel quel dans un dossier ISO 26262 / IEC 62304 /
EN 50128 / UNECE R155 (gabarit : `docs/AUDIT-TEMPLATE.md`).

## Ce que Tamis a prouvé — et ce qu'il ne couvre pas

| Prouvé | Comment |
|---|---|
| Chemin d'attaque atteignable | `reach` + contre-exemple rejouable |
| Non-interférence après correctif | `proof` (parcours exhaustif, compteur publié) |
| Minimisation sans changement de comportement | `minimize` (équivalence toute-paire) |
| Non-régression de privilège | `diff` sémantique |

**Angles morts bornés (honnêteté, TAM-21)** : flux IPC indirects (`MsgSend`/`ConnectAttach`),
permissions de fichiers, transitions root→non-root — la documentation QNX elle-même les dit
invisibles à l'analyse de politique. Tamis les **signale comme non couverts**, il ne prétend
pas les résoudre.

## Pour aller plus loin

- `make test` (810 vérifications), `make sanitize` (ASan/UBSan), `make mutants` (12 mutants tués).
- Les 100 idées d'approfondissement : `docs/100-idees.md`.
- Le contrat gelé des formats : `CONTRACT.md`.
