<!-- SPDX-License-Identifier: MIT -->
# La licence QNX SDP 8.0 — ce qu'elle débloque, et comment l'exploiter

La licence gratuite est **le seul maillon manquant** du projet. Tout le reste est en place et
testé. Ce document dit précisément ce qu'elle apporte, comment l'obtenir, et ce qu'il faut
faire dans la minute qui suit.

## Ce que la licence débloque

| | Sans SDP (aujourd'hui) | Avec SDP 8.0 |
|---|---|---|
| Grammaire `secpol` | relevée depuis l'inventaire + attestée sur un vrai QNX 7.1 (16 points `À VÉRIFIER`) | **extraite automatiquement** de la doc installée, production par production |
| Parseur du vrai langage | pas écrit (mode sous-ensemble strict sur un format de fixture) | même modèle, même moteur : il n'y a qu'à brancher le lexer |
| Différentiel `secpolcompile` (TAM-12) | harnais écrit, sort `3` (prérequis manquant) | **accord vérifié** avec le compilateur officiel, politique par politique |
| Boucle *prédit ↔ observé* | impossible : aucun `secpol.bin` à pousser | Tamis prédit un refus, QNX l'exécute et le confirme |
| `secpolgenerate` | absent | audit du diff incrémental de la politique générée |
| Faisabilité des jalons | — | jalons du 13/11 au 28/11 (cahier §4) |

> Le reste du projet **ne dépend pas** de la licence : socle C, 6 moteurs, CLI, site bilingue,
> jumeaux Python/JS, fuzz, mutants, banc QEMU réel. La licence est un accélérateur et une
> preuve supplémentaire, pas une condition d'existence.

## Obtenir la licence (10 minutes, action humaine)

1. Créer un compte **myQNX** : <https://www.qnx.com/developers/download/>
   (e-mail + mot de passe ; c'est le seul compte à créer, je ne peux pas le faire à ta place).
2. Télécharger **QNX SDP 8.0 for Linux (host)** depuis la même page.
3. L'installer, par exemple dans `~/qnx800` (ou ailleurs, il suffira d'exporter `QNX_SDP`).
4. Exposer les outils hôtes :

```sh
export QNX_SDP=~/qnx800
export PATH="$QNX_SDP/host/linux/x86_64/usr/bin:$PATH"   # secpolcompile, secpolgenerate…
```

## Dans la minute qui suit : une seule commande

```sh
bash banc/sdp.sh
```

Le script :

1. **vérifie** que le SDP est là et que `secpolcompile`, `secpolgenerate`, `secpolpush` existent ;
2. **localise** la documentation `secpol_language.html` installée avec le SDP ;
3. **extrait la grammaire** automatiquement dans `docs/GRAMMAR-auto.md` (chaque production avec
   sa page source + le vocabulaire réellement employé, donc l'orthographe exacte des
   modificateurs — à relire puis à fusionner dans `docs/GRAMMAR.md`) ;
4. **relance** le différentiel oracle contre `secpolcompile` (`oracle/run_oracle.sh`).

L'extracteur est **déjà validé** sans SDP, sur une page HTML synthétique
(`tests/test_sdp.sh`, 15 vérifications) : il n'est pas une promesse, c'est du code testé.

## Deux points d'attention

**Le vocabulaire peut avoir bougé entre 7.1 et 8.0.** Sur le binaire QNX **7.1** réel, les
modificateurs sont `allow`, `deny`, `inherit`, `no-inherit` (avec tiret), `nonroot`, `root`,
`lock`, `subrange` — ce qui **corrige** la lecture indirecte de la documentation
(`noinherit`, `unlock`). L'extraction automatique depuis le SDP 8.0 arbitrera
définitivement ; tout écart avec `preuve/qnx71/syntaxe-attenstee.txt` sera un signal fort.

**Le SDP est volumineux** (plusieurs Go). L'extraction ne lit que le HTML de la doc
(`htm/en/topic/com.qnx.doc.security.secpol`), pas le reste.

## Si la licence n'arrive pas

Le projet reste présentable tel quel : la grammaire est déjà attestée sur un **vrai** système
QNX (7.1, banc permanent sur le r630), le parseur peut être écrit sur le sous-ensemble attesté
(`;`, `derive_type`, `allow_attach`, `allow_link`, les 8 modificateurs, les plages
`début-fin`) en **mode sous-ensemble strict** — c'est-à-dire en refusant explicitement ce
qu'il ne sait pas parser, comme le veut le cahier. Ce qui manque reste la **preuve** contre
`secpolcompile` et la boucle *prédit ↔ observé*.
