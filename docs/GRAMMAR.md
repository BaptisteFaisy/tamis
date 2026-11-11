<!-- SPDX-License-Identifier: MIT -->
# Tamis — GRAMMAIRE du langage de politique de sécurité QNX (`secpol`)

**Lot L0 — essai E1 du cahier des charges.** Statut : **relevé documenté, NON gelé**.

> ⚠️ **Avertissement d'honnêteté (À LIRE AVANT DE CODER LE PARSEUR).**
> La documentation officielle QNX (`secpol_language.html`, `using_security_policy_types.html`,
> `named_range_file.html`, `abilities_reference.html`) était **inaccessible en ligne** le
> 11/11/2026 (HTTP 404 sur toutes les variantes d'URL) et le **SDP 8.0 n'est pas installé**
> (licence gratuite à demander — action humaine, voir `docs/DECISIONS.md`).
> Ce document est donc relevé depuis **`qnx-inventaire-officiel.md`** (dépôt `qnx-dossier`),
> qui cite les pages sources mais ne reproduit pas la syntaxe terminale exacte.
> **Chaque production ci-dessous est marquée `[SOURCÉ]` (forme attestée) ou `À VÉRIFIER`
> (terminaux exacts : séparateurs `;`, placement des `{}`, orthographe des mots-clés,
> ordre des champs).** Aucune syntaxe n'est inventée : ce qui n'est pas attesté est dit.
> **Le parseur (`core/src/lex.c`, `ast.c`) ne doit PAS être écrit avant que ce fichier
> soit gelé** par lecture page à page de la doc officielle (SDP installé ou doc en ligne).

---

## 1. Chaîne de compilation officielle `[SOURCÉ]`

```
secpol.txt  →  secpolcompile  →  secpol.bin  →  (inséré dans l'IFS)  →  secpolpush
```
Source : `secpol_language.html`, `security_policies.html`. `secpolcompile` est un
utilitaire **hôte** — c'est ce qui rend le différentiel Tamis possible sans cible.

## 2. Éléments de déclaration `[SOURCÉ]`

Source : `secpol_language.html`, `named_range_file.html`, `using_security_policy_types.html`.

| Élément | Rôle documenté | Syntaxe terminale |
|---|---|---|
| `type` | déclare un type de sécurité (nom) | `À VÉRIFIER` : `type <nom>;` ? |
| `attribute` | groupe de types (attribut nommé) | `À VÉRIFIER` : `attribute <nom> { <types> };` ? |
| `class` | permissions custom (objets non-canaux) | `À VÉRIFIER` : `class <nom> { <perms> };` ? |
| `range` | plages nommées résolues au boot | `À VÉRIFIER` : voir §6 |
| `default_rules` | bloc de règles par défaut | `À VÉRIFIER` |

**Fait attesté et décisif** : *« Currently, the only objects that can be typed are channels »*
(`using_security_policy_types.html`, trou S15). Tout autre objet typé passe par une `class`
custom et `secpol_get_permission()` / `secpol_check_permission()` côté resource manager.

## 3. Règles d'accès `[SOURCÉ]`

Source : `secpol_language.html`. Formes attestées (la ponctuation exacte reste `À VÉRIFIER`) :

```
allow <src> <dst>:<ability>{ <plages> }        # accord d'ability, avec sous-plages possibles
allow <src> <dst>:channel connect              # connexion à un canal
allow_attach <type> <chemin> [<ftype>:] [<type_canal>]   # attachage de chemin (wildcards)
allow_link ...                                 # lien (forme exacte À VÉRIFIER)
default_spawn_type ...                         # type par défaut au spawn (jamais généré, trou S7)
derive_type ...                                # dérivation de type
mount_point <uid>:<gid> <perms/ACL> <chemins>  # point de montage
```

- **Wildcards attestés** : `*` et `...` dans les chemins (`allow_attach`). `[SOURCÉ]`
- **Modificateurs attestés** sur `allow` : `nonroot`, `unlock`, `noinherit`, `gain_priv`.
  `[SOURCÉ]` — placement exact (avant/après, séparateur) `À VÉRIFIER`.
- `gain_priv` : acquis via le graphe `settypeid` ; la doc prescrit l'essai-erreur pour
  savoir quel type apporte quel privilège (trou S8). `[SOURCÉ]`

## 4. Plages nommées (`range`) `[SOURCÉ partiel]`

Source : `named_range_file.html`, `secpol_language.html`.
- Les abilities peuvent avoir des **sous-plages numériques** ; `secpolcompile` **fusionne les
  sous-plages contiguës**. `[SOURCÉ]`
- Une plage nommée est résolue **au boot** via un fichier de la forme
  `ability:range_name:syspage:/…` (le chemin exact est `À VÉRIFIER`). `[SOURCÉ partiel]`
- **Fait attesté (trou S13)** : *« If any range can't be resolved … the policy is not pushed
  and it is likely the system won't boot »* ; `secpolpush` ne fusionne pas les plages
  adjacentes. D'où l'exigence TAM-13 (validation à froid des plages).

## 5. Abilities `[SOURCÉ]`

Source : `abilities.html`, `abilities_reference.html`.
- **~70 abilities statiques** (procmgr). Chaque processus a **2 jeux** : root et non-root.
- **Custom abilities** dynamiques, par nom (préfixées) : voir `politique/abilities.tsv`.
- Inventaire complet et ce que chacune autorise : **`politique/abilities.tsv`** (une ligne
  par ability, avec sa portée et sa source).

## 6. Ce que le compilateur vérifie déjà `[SOURCÉ]`

Source : `secpol_language.html`, `using_security_policy_types.html`.
- Types non déclarés → erreur.
- Erreurs `gain_priv` : avertissements/erreurs si privilèges acquis par `settypeid` non déclarés.
- Fusion des sous-plages contiguës.

C'est le **périmètre à ne pas doubler** : Tamis se branche **après** `secpolcompile`
(analyse sémantique : accessibilité, minimalité, diff), pas sur la validation syntaxique
de base que le compilateur fait déjà.

## 7. Statut — ce qui est sourcé vs À VÉRIFIER

| Production | Statut |
|---|---|
| Chaîne `secpol.txt → secpolcompile → secpol.bin → secpolpush` | `[SOURCÉ]` |
| Existence de `type`, `attribute`, `class`, `range`, `default_rules` | `[SOURCÉ]` |
| Existence de `allow`, `allow_attach`, `allow_link`, `default_spawn_type`, `derive_type`, `mount_point` | `[SOURCÉ]` |
| Modificateurs `nonroot`, `unlock`, `noinherit`, `gain_priv` | `[SOURCÉ]` |
| Wildcards `*` et `...` | `[SOURCÉ]` |
| Fusion des sous-plages contiguës par `secpolcompile` | `[SOURCÉ]` |
| **Terminaux exacts** (`;`, `{}`, ordre des champs, orthographe) | **À VÉRIFIER page à page** |
| Forme exacte de `range` / fichier `ability:range_name:syspage:/…` | **À VÉRIFIER** |
| Forme exacte de `allow_link`, `derive_type`, `default_spawn_type` | **À VÉRIFIER** |
| Grammaire EBNF complète et non ambiguë | **À VÉRIFIER** — bloque le parseur |

## 8. Prochaine étape (E1 à trancher)

1. Obtenir le SDP 8.0 (licence gratuite myQNX — action humaine) **ou** un accès hors-ligne
   à la doc (`secpol_language.html` et pages liées).
2. Lire **page à page** et remplacer chaque `À VÉRIFIER` par la production exacte + la page.
3. **Geler** ce fichier, puis seulement écrire `core/src/lex.c` + `ast.c` (lot L1).
4. Croiser avec une politique d'exemple publique (essai E3) si disponible.

Jusqu'au gel : Tamis applique le **mode sous-ensemble strict** (voir `docs/DECISIONS.md`) —
il refuse d'analyser ce qu'il ne sait pas parser, et le dit (`TAMIS_PARSE_ERROR` avec
position et cause), **jamais** d'analyse approximative.
