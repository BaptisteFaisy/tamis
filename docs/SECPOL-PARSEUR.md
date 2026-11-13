<!-- SPDX-License-Identifier: MIT -->
# Contrat du parseur secpol (lot L1)

**Statut : GELÉ le 12/11/2026.** Ce fichier est le contrat unique que le parseur C
(`core/src/`), le jumeau JS (`site/`) et le différentiel oracle suivent. La grammaire de
référence est `docs/GRAMMAR.md` (elle-même gelée depuis la doc officielle du SDP 8.0 et
croisée avec `secpolcompile`).

## 1. Principe

- **Sous-ensemble strict.** Le parseur accepte exactement les productions listées §3. Tout le
  reste produit `TAMIS_PARSE_ERROR` avec **ligne** et **colonne** (1-based) et le premier
  jeton fautif. Jamais d'analyse approximative.
- Le parseur produit le **même `tamis_model`** que l'ancien parseur provisoire : les moteurs
  (reach, proof, lint, minimize, diff, smt) ne changent pas d'une ligne.

## 2. Lexique

| Jeton | Règle |
|---|---|
| IDENT | `[A-Za-z_][A-Za-z0-9_]*` |
| NOM | IDENT, plus `self` (mot-clé mais autorisé comme nom de cible) |
| CHEMIN | un `mot` composé de caractères autres que `;{}` et l'espace : `[^;{}[:space:]]+` (ex. `/dev/socket/*`, `mount+:/system`, `...`) |
| NOMBRE | `[0-9]+` ou `0x[0-9a-fA-F]+` |
| PONCT | `;` `{` `}` `:` `,` `(` `)` `+` `*` |
| COMMENTAIRE | de `#` à la fin de ligne |

Le découpage en mots est fait sur les **espaces** ; les signes de ponctuation (`;{}:,()+-`)
sont reconnus **même accolés** à un mot (ex. `self:ability` → `self` `:` `ability` ; `};` →
`}` `;`).

## 3. Productions acceptées

```
type IDENT ;
range IDENT ;
attribute IDENT ;
class IDENT { IDENT+ } ;
allow NOM NOM : channel connect ;                       # (a) connexion de canal
allow NOM NOM : ability { bloc_ability } ;              # (b) abilities, cible normalement `self`
allow_attach IDENT CHEMIN [ CHEMIN ] ;                  # (c) attachage, 1 ou 2 arguments
allow_attach IDENT { bloc_ability } ;                   # (d) bloc (1 argument en-tête)
allow_link IDENT CHEMIN ;                               # (e)
allow_link IDENT { bloc_ability } ;                    # (f)
derive_type NOM NOM NOM ;                              # (g) forme « type1 name type2 »
derive_type { NOM+ } { bloc_ability } NOM ;             # (h) forme groupée
default_spawn_type NOM NOM ;
mount_point NOMBRE : NOMBRE { bloc_ability } ;
```

`bloc_ability` = suite de lignes, chacune étant soit un **modificateur** (`nonroot`,
`unlock`, `noinherit`, `inherit`, `deny`, `allow`, `root`, `subrange`), soit un
**élément d'ability** :

```
ability_item := MOT [ ':' liste_args ]
MOT          := IDENT | IDENT '/' IDENT ('/' IDENT)*       # ex. network/bind/privport, file:read
liste_args   := arg (',' arg)*
arg          := NOMBRE | NOMBRE '-' NOMBRE | NOMBRE '-' | NOM           # « 96- » = borne ouverte
```

## 4. Correspondance `tamis_model`

| Production | Effet sur le modèle |
|---|---|
| `type NOM ;` | ajoute le nœud NOM (les doublons → erreur de déclaration) |
| `range NOM ;` | enregistré (aucune arête ; prépare TAM-13) |
| `attribute NOM ;` / `class …` | enregistré (aucune arête) |
| `allow X self:ability { … settypeid:Y … }` | **arête X → Y** (transition de type). `Y` **doit** être un type déclaré, sinon erreur |
| `allow X self:ability { … gain_priv:Y … }` | si `Y` est un type déclaré → **arête X → Y** ; si `Y` contient `:` (permission custom, ex. `file:read:file1_t`) → ability seule, **pas d'arête** ; sinon (`Y` = nom d'ability, ex. `pathspace`) → ability seule, **pas d'arête**. Le compilateur officiel n'en fait qu'un *warning* (« unneeded gain_priv abilities ») — Tamis n'en fait donc pas une erreur |
| `allow X Y:channel connect;` | **arête X → Y** (accès au canal). `Y` doit être un type déclaré |
| tout autre `ability_item` (ex. `mem_phys:…`, `able_create`) | pas d'arête ; la règle est conservée (pour lint) |
| `default_spawn_type …` | `model.has_default_spawn = 1` |
| `allow_attach`, `allow_link`, `derive_type`, `mount_point` | enregistrés (aucune arête pour l'instant) |

Chaque arête conserve `line` (ligne de la règle `allow`) et un label : le nom de l'ability de
transition (`settypeid`, `gain_priv`) ou `channel` — utile pour `diff`.

## 5. Erreurs (toutes `TAMIS_PARSE_ERROR`, position précise)

- jeton inattendu, production incomplète (`;` manquant, `}` manquant) ;
- `type` déjà déclaré ;
- type référencé (`settypeid:Y`, `gain_priv:Y`, `allow X Y:…`) non déclaré ;
- `ability_item` mal formé (`setuid:` sans argument, plage `-` seule, `0x` sans chiffres).

## 6. Le site et le jumeau navigateur

Le jumeau JS est découpé **comme le C**, pour la même raison (lisibilité et règle
< 500 lignes/fichier) :

| Fichier JS | Jumeau C |
|---|---|
| `site/tamis-secpol-abil.js` | `core/src/secpol_abil.c` — vocabulaire fermé (62 statiques, 30 à sous-plage, 3 modificateurs) |
| `site/tamis-secpol-decl.js` | `core/src/secpol_decl.c` — classes, permissions, attributs, plages, relabels |
| `site/tamis-secpol-lex.js` | `core/src/secpol_lex.c` — lexique et positions |
| `site/tamis-secpol-prod.js` | `core/src/secpol_parse.c` + pilote de `secpol.c` |
| `site/tamis-secpol.js` | `tamis_model_parse_secpol()` — API `{ model, error }` |

**Détection de format.** Le CLI choisit sur l'extension (`.secpol`), le site sur le
contenu : toute politique qui contient un `;` hors commentaire est du vrai `secpol`, sinon
c'est le format provisoire `.mpf`. Dans les deux cas, un refus affiche
`TAMIS_PARSE_ERROR` avec **ligne:colonne** et la **cause** — jamais une analyse partielle.

**Preuve d'équivalence.** `tests/test_js.sh` rejoue les 42 politiques du corpus secpol
(`fixtures/secpol-cas/`) et la politique QNX 8.0 à travers les deux implémentations : même
acceptation, même message, même position. Une divergence est un bug, pas une nuance.