# Tamis 🦀

> **Site de démonstration : https://baptistefaisy.github.io/tamis/** —
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
build/tamis view     fixtures/attaque.mpf --src app_tiers --dst pompe_critique > /tmp/g.json
```
Ouvre ensuite `view/index.html` et colle `/tmp/g.json` : le chemin d'attaque s'affiche en rouge.
Tutoriel pas à pas : [`docs/codelab/01-auditer-une-politique.md`](docs/codelab/01-auditer-une-politique.md).

> Les politiques d'exemple sont au **format provisoire** (`type`/`allow`/`untyped`). Le parseur
> du vrai langage `secpol` se branchera sur le même modèle dès la grammaire gelée.

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
| Grammaire `secpol` exacte (`docs/GRAMMAR.md`) | 🚧 relevée, À VÉRIFIER page à page (doc 404, SDP absent) |
| Parseur `secpol` réel (lot L1) | ⏳ attend la grammaire gelée |
| Différentiel contre `secpolcompile` (`oracle/`) | ⏳ harnais prêt, attend le SDP hôte |

**810 vérifications** vertes (`make test`), garde de taille OK, sanitizeurs clean.

## Build et test

Aucune dépendance à installer : `gcc` (C11) et `python3` (standard) suffisent.
**Tout tourne sans QNX, sans QEMU, sans licence** — le différentiel contre `secpolcompile`
est un plus, pas un prérequis.

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

**https://baptistefaisy.github.io/tamis/** — publié sur GitHub Pages depuis `site/`
(workflow `pages.yml`), **bilingue FR/EN** (bascule en haut à droite, anglais par défaut pour
les juges). Sept onglets :

| Onglet | Contenu |
|---|---|
| **Playground** | toutes les commandes exécutées en direct par le jumeau JS : `lint`, `reach`, `proof`, `explain`, `minimize`, `diff`, graphe de privilèges interactif, sortie texte / JSON / Markdown d'audit, badge de code de retour CI |
| **Features** | 21 fonctionnalités : commande, apport, exigence TAM, état |
| **Evidence** | les 8 niveaux de vérification + la traçabilité des **24 exigences** du cahier |
| **Submission** | le dossier de soumission complet : pitch, catégorie, bonus, **preuve d'usage QNX**, comment c'est construit, instructions pour les juges, script vidéo 3 min, liens |
| **Codelab** | le tutoriel 20 min en 11 étapes, avec les commandes et leurs sorties réelles |
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

## Licence

MIT — voir [`LICENSE`](LICENSE).
