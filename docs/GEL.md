<!-- SPDX-License-Identifier: MIT -->
# Gel de périmètre — décision et liste des fonctions livrées

**Décision : actionné le 12/11/2026.** À partir du **25/11/2026** : plus aucune
fonctionnalité nouvelle, plus aucune dépendance externe, plus aucune ligne de cœur sauf
correctif bloquant voté explicitement.

## Règles (opposables)

1. **Aucune nouvelle sous-commande CLI** après le gel (la liste ci-dessous fait foi).
2. **Aucune nouvelle dépendance externe** : `core/` reste C11 pur, `host/`/`tools/` Python 3
   stdlib, `site/` JS sans bibliothèque. Toute exception exige une note dans `docs/DECISIONS.md`.
3. **Aucun nouveau verdict** (les 13 codes sont figés dans `CONTRACT.md`).
4. **Aucun fichier public supprimé ou renommé** après le gel (les liens de `docs/top10-consolidation.md`
   et du formulaire Devpost doivent rester valides).
5. Le **format JSON** (`CONTRACT.md` §5.2) est le contrat de démonstration opposable : il ne
   change plus.

Vérification automatique : `tools/check_perimetre.sh`.

## Liste exacte des fonctions livrées (au gel)

### CLI `tamis` — 9 sous-commandes
| Commande | Rôle |
|---|---|
| `lint` | moindre privilège : processus non typés, `default_spawn_type` absent (TAM-06) |
| `reach` | accessibilité transitive + chemin complet (TAM-04) |
| `proof` | certificat de non-interférence, parcours exhaustif + compteur (TAM-05/20) |
| `explain` | explication pédagogique + correctif minimal + angles morts (TAM-21) |
| `minimize` | règles redondantes + preuve d'équivalence toute-paire (TAM-07) |
| `diff` | élargissements / réductions de privilège (TAM-08) |
| `smt` | preuve SMT-LIB2 (Z3 l'exécute si présent) |
| `view` | graphe de privilèges en JSON |
| `version` | version |

### Verdicts — 13 codes
`TAMIS_OK`, `TAMIS_PARSE_ERROR`, `TAMIS_UNTYPED_PROCESS`, `TAMIS_MISSING_DEFAULT_SPAWN`,
`TAMIS_DEAD_RULE`, `TAMIS_REDUNDANT_RULE`, `TAMIS_OVERBROAD_ABILITY`, `TAMIS_RANGE_INVALID`,
`TAMIS_PATH_FOUND`, `TAMIS_INTERFERENCE_POSSIBLE`, `TAMIS_PRIVILEGE_WIDENED`,
`TAMIS_PRIVILEGE_NARROWED`, `TAMIS_INCONSISTENT`.

### Formats de sortie — 3
texte · JSON (une ligne par verdict) · Markdown d'audit (empreinte SHA-256, horodatage, version).

### Surfaces
- **Trois implémentations** : cœur C (autoritaire), jumeau Python, jumeau JS (site) — rendu
  identique octet pour octet, vérifié par différentiel.
- **Site GitHub Pages** bilingue FR/EN, 7 onglets : Playground, Fonctionnalités, Preuves,
  Soumission, Codelab, 100 idées, Grammaire & preuve QNX (+ Banc réel).
- **Outillage** : pont local (127.0.0.1, jeton, listes blanches) vers le vrai C et la VM QNX,
  banc QEMU QNX 7.1 sur le serveur r630, chaîne SDP (`banc/sdp.sh`, `tools/extract_grammar.py`).

### Preuves publiées
`preuve/qnx71/` (banc réel 7.1) · `preuve/qnx8/` (compilateur officiel 8.0 : accepte notre
politique, 0 erreur) · différentiel C↔Python↔JS · fuzz 9 000 · 12 mutants · 9 969 vérifications.

## Dépendances externes (inventaire figé)

| Cible | Dépendances |
|---|---|
| `core/` | **aucune** — C11, zéro allocation hors arène |
| `host/`, `tools/` | Python 3 stdlib uniquement |
| `site/` | aucun module ; HTML/CSS/JS vanilla |
| Build | `gcc`, `make`, `python3` (présents partout) |
| Outils **optionnels** | `z3` (preuve SMT), QEMU + SDP QNX (banc/oracle) — jamais requis pour `make test` |

## Ce qui n'est **pas** gelé

Les documents (docs/*), les tests, le site contenu (texte), et l'outillage de preuve. Le
code du **parseur `secpol` réel** (lot L1) reste le seul chantier fonctionnel autorisé
après le 25/11, car la grammaire vient d'être gelée ; il doit respecter les 5 règles
ci-dessus (mêmes 13 verdicts, mêmes formats, C11 pur).