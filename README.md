# Tamis 🦀

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

## État

| Élément | État |
|---|---|
| Contrat gelé (`CONTRACT.md`) | ✅ 13 verdicts, formats texte/JSON/Markdown, API C |
| Socle verdicts + rapports (`core/`) | 🚧 en cours |
| Jumeau Python (`host/`) | 🚧 en cours |
| Grammaire `secpol` (`docs/GRAMMAR.md`) | 🚧 relevée depuis la doc, à vérifier page à page (SDP) |
| Parseur / modèle / accessibilité | ⏳ attend la grammaire gelée |
| Différentiel contre `secpolcompile` (`oracle/`) | ⏳ attend le SDP hôte |

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
core/      C11 pur (ni horloge, ni fichier, ni socket, ni état global) — le noyau d'analyse
host/      Python 3 standard — jumeau de vérification indépendant + CLI
fixtures/  corpus de politiques (minimales, réalistes, fautives)
oracle/    harnais de test différentiel contre secpolcompile (hôte, optionnel sans SDP)
tests/     différentiel C↔Python, fuzz, banque de cas fautifs, mutants, gardes
docs/      GRAMMAR.md, DECISIONS.md, VALIDATION.md, AUDIT-TEMPLATE.md
```

La double implémentation (C dans `core/`, Python dans `host/`) n'est pas une duplication :
c'est un **jumeau de vérification** indépendant, comparé octet par octet sur ≥ 200 cas.

## Licence

MIT — voir [`LICENSE`](LICENSE).
