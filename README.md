# Tamis 🦀

> **Site de démonstration : https://tradoooo.github.io/tamis/** —
> toutes les fonctionnalités sont **essayables en direct dans le navigateur**
> (playground), zéro installation.

> *QNX génère et surveille les politiques de sécurité ; QNX demande à l'intégrateur de les
> relire, les fusionner et les justifier **à la main**. Tamis outille exactement cette partie,
> et le prouve en se testant par différentiel contre `secpolcompile`.*

**Tamis** est un outil hôte d'**analyse statique et de preuve** des politiques de sécurité
QNX 8.0 (`secpol`). Avant déploiement, il répond à quatre questions que la chaîne officielle
ne pose pas :

1. Cette règle est-elle **atteignable** ?
2. Ce type peut-il, par une chaîne d'intermédiaires, atteindre ce type **critique** ?
3. Quelles règles sont **mortes** / redondantes ?
4. Cette modification **élargit-elle** un privilège ?

Projet candidat au **QNX Global Hackathon 2026** (dépôt le 15 décembre), catégorie
*Developer Tooling & Host Tools*, visant le bonus *Best use of QNX-specific Technologies*.

## Pourquoi (les trous documentés)

La documentation officielle QNX reconnaît elle-même que tout l'aval de `secpolgenerate` est
manuel : revue des règles inutiles, fusion, stabilisation, vérification du typage, analyse
d'élévation de privilèges par essai-erreur. **Aucune analyse statique avant déploiement.**
Tamis occupe cet espace — sans jamais doubler `secpolgenerate`/`secpolcompile`/`secpolpush`.

## Démarrage rapide

```
make cli                                              # construit build/tamis
build/tamis lint     fixtures/attaque.mpf             # moindre privilège
build/tamis reach    fixtures/attaque.mpf --src app_tiers --dst pompe_critique
build/tamis explain  fixtures/attaque.mpf --src app_tiers --dst pompe_critique
build/tamis proof    fixtures/durcie.mpf  --src app_tiers --dst pompe_critique
build/tamis minimize fixtures/attaque.mpf
build/tamis diff     fixtures/attaque.mpf fixtures/durcie.mpf
build/tamis smt      fixtures/attaque.mpf --src app_tiers --dst pompe_critique   # preuve SMT-LIB2
build/tamis view     fixtures/attaque.mpf --src app_tiers --dst pompe_critique > /tmp/g.json
```
Ouvre ensuite `view/index.html` et colle `/tmp/g.json` : le chemin d'attaque s'affiche en rouge.
Tutoriel pas à pas : [`docs/codelab/01-auditer-une-politique.md`](docs/codelab/01-auditer-une-politique.md).

### La preuve par solveur

`tamis smt` émet l'accessibilité sous forme **SMT-LIB2** (clauses de Horn) : la question
« src atteint-il dst ? » devient une satisfiabilité. Avec `z3` installé, la commande
l'exécute et tranche :

```
$ tamis smt fixtures/attaque.mpf --src app_tiers --dst pompe_critique
…
(assert (not r_0_3))
(check-sat)
; solveur : unsat
; → le chemin EXISTE (UNSAT : r_src_dst est implique)
```
Un `sat` prouve au contraire l'absence de chemin. L'encodage a été **validé par Z3 4.13.3** :
`unsat` pour le chemin qui existe, `sat` pour l'absence de chemin. L'encodage est rejouable
hors ligne par un tiers (`z3 -smt2 fichier.smt2`) — c'est le critère « implémentation non
triviale » du règlement.

> Le **vrai langage `secpol`** est accepté : `build/tamis` choisit le parseur sur l'extension
> (`.secpol`), et le site le détecte sur le contenu. Les fixtures `.mpf` restent au format
> provisoire pour les démonstrations en place.

## État

| Élément | État |
|---|---|
| Contrat gelé (`CONTRACT.md`) | ✅ 13 verdicts, formats texte/JSON/Markdown, API C |
| Socle verdicts + rapports (`core/`) | ✅ 287 vérifications, ASan/UBSan clean |
| Jumeau Python (`host/`) + différentiel C↔Python | ✅ 0 divergence (250/300/500 verdicts) |
| Couche modèle + parseur (format provisoire) | ✅ `core/model.c` |
| Moteurs : reach, proof, lint, minimize, diff, explain | ✅ TAM-04/05/06/07/08 |
| CLI `tamis` + visualiseur HTML du graphe | ✅ TAM-23 |
| Fuzz (ASan/UBSan) + mutants | ✅ 9 000 entrées, 12 mutants tués |
| Codelab + rapport d'audit + angles morts | ✅ |
| Grammaire `secpol` exacte (`docs/GRAMMAR.md`) | ✅ **gelée** — 76 productions extraites du SDP 8.0, croisées avec `secpolcompile` |
| Parseur `secpol` réel | ✅ C (`core/src/secpol*.c`) + jumeau navigateur (`site/tamis-secpol*.js`) : mêmes verdicts, mêmes refus, mêmes positions |
| Différentiel contre `secpolcompile` (`oracle/`) | ✅ exécuté sur le SDP 8.0 : accord sur les 43 politiques (42 du corpus + la politique QNX 8.0) |
| Invité QNX 8.0 en direct | ✅ le cœur C recompilé par `qcc` analyse une vraie politique dans l'invité (QEMU) |
| Preuves rejouées dans la vraie cible | ✅ service public : liste fermée d'étapes exécutées **dans l'invité QNX 8.0**, sortie comparée octet pour octet à celle du poste |

**10 247 vérifications** vertes (`make test` : 537 C + 9 710 hôtes, dont 61 du
différentiel C↔JS qui couvre les 42 cas secpol), garde de taille OK, sanitizeurs clean.
Ce nombre sort de `make test` — il n'est jamais recopié à la main (règle 6 du rétroplanning).

## Build et test

Aucune dépendance obligatoire : `gcc` (C11) et `python3` (standard) suffisent pour tout le
noyau. `node` (facultatif) active le différentiel C↔JS et le test du site ; sans lui deux
suites s'annoncent « ignorées », le reste passe. **Tout tourne sans QNX, sans QEMU, sans
licence** — le différentiel contre `secpolcompile` est un plus, pas un prérequis.

```
make test       # construit et lance tous les tests (C + Python + différentiel)
make sanitize   # relance les tests C sous ASan/UBSan
make garde      # < 500 lignes/fichier, < 50 lignes/fonction
make clean
```

## Architecture

```
core/      C11 pur (ni horloge, ni fichier, ni socket, ni état global, aucune allocation :
           l'appelant fournit une arène) — le noyau d'analyse : model, reach, proof, lint,
           minimize, diff, explain, verdict, rapport
host/      Python 3 standard — jumeau de vérification indépendant du rendu
site/      jumeau navigateur (JS sans dépendance) — playground publié sur GitHub Pages
tools/     CLI `tamis`, sha256, fuzzer, générateur du site
fixtures/  corpus de politiques (format provisoire ; le vrai secpol viendra s'y substituer)
oracle/    harnais de test différentiel contre secpolcompile (hôte, optionnel sans SDP)
tests/     différentiels C↔Python et C↔JS, fuzz, mutants, CLI, site, gardes
docs/      GRAMMAR.md, DECISIONS.md, VALIDATION.md, AUDIT-TEMPLATE.md, 100-idees.md, codelab/
politique/ abilities.tsv (75 abilities relevées dans la doc QNX)
```

**Trois implémentations indépendantes, un seul contrat** : le cœur C (autoritaire), le jumeau
Python et le jumeau navigateur JS rendent le même JSON **octet pour octet**. C'est vérifié en
continu par `tests/test_differentiel.sh` (C↔Python, 250/300/500 verdicts aléatoires) et
`tests/test_js.sh` (C↔JS, 13 scénarios d'analyse réels). Une divergence est un bug, pas une
différence d'opinion.

## Site de démonstration & dossier de soumission

**https://tradoooo.github.io/tamis/** — publié sur GitHub Pages depuis `site/`
(workflow `pages.yml`), **bilingue FR/EN** (bascule en haut à droite, anglais par défaut pour
les juges). Sept onglets :

| Onglet | Contenu |
|---|---|
| **Playground** | toutes les commandes exécutées en direct par le jumeau JS : `lint`, `reach`, `proof`, `explain`, `minimize`, `diff`, graphe de privilèges interactif, sortie texte / JSON / Markdown d'audit, badge de code de retour CI |
| **Features** | 21 fonctionnalités : commande, apport, exigence TAM, état |
| **Evidence** | les 8 niveaux de vérification + la traçabilité des **24 exigences** du cahier |
| **Submission** | le dossier de soumission complet : pitch, catégorie, bonus, **preuve d'usage QNX**, comment c'est construit, instructions pour les juges, script vidéo 3 min, liens |
| **Codelab** | le tutoriel 20 min en 14 étapes, **rejouable dans le navigateur** (bouton « Rejouer ici ») : les 3 dernières étapes portent sur le vrai langage `secpol` QNX 8.0 |
| **100 ideas** | la feuille de route, ✓ sur ce qui est déjà livré |
| **Grammar & contract** | ce qui est gelé, ce qui est `À VÉRIFIER`, et pourquoi |

**Une seule source de vérité** : le contenu vit dans `site/content.json` (+ `fixtures/` et
`docs/`), et `tools/build_site.py` le rend dans les deux langues entre marqueurs. La CI
vérifie l'absence de dérive — le site ne peut pas contredire le dépôt.
`tools/build_devpost.py` génère depuis la même source `docs/devpost-form.md`, le formulaire
Devpost **prêt à coller** (13 champs, en anglais).

Le câblage de la page est testé **sans navigateur** (`tests/site_smoke.js`, mini-DOM :
46 vérifications — démarrage, 7 commandes, 3 formats, graphe, onglets, bascule de langue,
erreur de syntaxe).

```
make site     # régénère site/ et docs/devpost-form.md depuis leurs sources
```

## Banc réel — le vrai cœur C et la vraie VM QNX derrière le site

Un site statique ne peut héberger ni le binaire C, ni un hyperviseur, ni l'image QNX de 6 Gio
(non redistribuable). Le **pont local** comble l'écart : il tourne sur ta machine, sur
`127.0.0.1` uniquement, et l'onglet **Banc réel** du site s'y connecte.

```
make cli
python3 tools/tamis-bridge.py     # imprime l'URL et le jeton
```

Il donne deux choses : le **différentiel en direct** (même politique analysée par le vrai
`build/tamis` et par le jumeau JS du navigateur, affichés côte à côte, avec verdict
identique/différent) et le **banc QNX réel** (démarrage/arrêt de la VM, commandes sur liste
blanche : `uname`, `secpol`, `secpolmonitor`, `libsecpol`, `hosttools`, `procnto`).

Sécurité en quatre couches : écoute `127.0.0.1` seule, **jeton aléatoire** obligatoire,
`Origin` contrôlée, aucune commande libre (listes blanches + `subprocess` sans shell + fichier
temporaire). Tout est vérifié par `tests/test_bridge.sh` (15 vérifications, dont « injoignable
depuis l'interface réseau »). Détails : [`docs/BANC-REEL.md`](docs/BANC-REEL.md).

Sans pont, le Playground reste entièrement fonctionnel via le jumeau JS.

## Invité QNX 8.0 en direct — le cœur C dans une vraie cible

Le cœur C **compile pour QNX 8.0** (`qcc -V gcc_ntox86_64`, `-Werror`) et tourne
**dans** un invité QNX 8.0 réel (QEMU, sur le serveur `r630`) : au démarrage, il
analyse la politique QNX 8.0 embarquée et affiche ses verdicts — les mêmes octets
que sur le poste. Vitrine en direct (écran VGA + console série + injection de
touches), vue sur le site (onglet **Invité QNX en direct**) et sur la page du
dossier QNX.

Construction et pièges : [`image/README.md`](image/README.md).

## Preuves rejouées dans la vraie cible — à la demande d'un juge

Le service `banc/service.py` exécute une **liste fermée** d'étapes **dans l'invité
QNX 8.0** (le vrai binaire QNX) et renvoie sa sortie, comparée **octet pour octet** à
celle du poste. Le site l'appelle depuis l'onglet **Invité QNX en direct** : un juge
clique, la preuve est produite dans la cible QNX, pas dans le navigateur.

```sh
curl -s http://127.0.0.1:8790/etat          # état de l'invité + étapes disponibles
curl -s http://127.0.0.1:8790/preuve/reach  # la preuve, dans l'invité
```

Exposé en HTTPS par `tailscale funnel` sous le montage public
`https://r630-debian.tail3a8bdf.ts.net:8443/tamis-api` (Funnel ne publie que
443, 8443, 10000 — les ports `:8092`/`:8094` ne s'ouvrent pas à internet ;
preuve et contrôle : [`docs/VITRINE-PUBLIQUE.md`](docs/VITRINE-PUBLIQUE.md),
`banc/prouve-public.sh`). L'invité QNX 8.0 ne tourne que
quand quelqu'un le regarde : la vitrine en est le seul propriétaire (arrêt après 5 min
d'inactivité), le service le réveille à la demande. Détails :
[`banc/LISEZMOI.md`](banc/LISEZMOI.md).

## 1 000 idées de consolidation

Catalogue complet : [`docs/1000-idees-consolidation.md`](docs/1000-idees-consolidation.md)
— **1 000 idées** réparties en 10 thèmes (cœur C, tests, spécificité QNX, preuve formelle,
performance, DX/CLI/distribution, documentation/codelab/i18n, démo/juges, pipeline/sécurité
de l'outillage, pilotage), sans doublon.

Sélection arbitrée et plan daté : [`docs/top10-consolidation.md`](docs/top10-consolidation.md).
Le **n°1 (encodeur SMT-LIB2)** est déjà livré et validé par un vrai solveur ; les n°5, 6, 7, 8
et 10 sont de l'écriture pure et se font dans la journée.

## Le dernier maillon — la licence QNX SDP 8.0 (10 min)

Tout le projet fonctionne **sans** la licence, mais elle est le seul maillon qui manque pour
deux choses : **figer la grammaire** du langage `secpol` et **prouver l'accord** avec le
compilateur officiel `secpolcompile` (TAM-12) — donc pour la boucle *prédit ↔ observé*, où
Tamis annonce un refus et QNX le confirme en s'exécutant.

Une fois le SDP installé (compte myQNX, licence gratuite) :

```sh
export QNX_SDP=~/qnx800
bash banc/sdp.sh
```

Le script vérifie les outils hôtes, **extrait automatiquement la grammaire** de la doc
installée (`tools/extract_grammar.py` → `docs/GRAMMAR-auto.md`, à relire puis fusionner) et
relance le différentiel oracle. L'extracteur est **déjà testé sans SDP**
(`tests/test_sdp.sh`, 15 vérifications sur une page HTML synthétique).

Détail et plan B : [`docs/SDP.md`](docs/SDP.md).

## Licence

MIT — voir [`LICENSE`](LICENSE).

Dépôt public : <https://github.com/Tradoooo/tamis> · Site de démonstration :
<https://tradoooo.github.io/tamis/>
