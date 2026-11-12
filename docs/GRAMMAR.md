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

> ✅ **Mise à jour du 12/11/2026 — preuve obtenue sur un système QNX réel.**
> À défaut de documentation, un vrai **QNX Neutrino 7.1.0** a été démarré en QEMU et interrogé
> (voir `preuve/qnx71/`). Cela lève une partie des incertitudes avec des artefacts vérifiables :
> **66 noms d'abilities** extraits du binaire, les **formes exactes** de `derive_type`,
> `allow_link` et `allow_attach`, le **terminateur `;`**, les **8 modificateurs réels**, le
> chemin `/proc/boot/secpol.bin`, et la **structure du blob compilé** (`SECPOL_BLOB_*`).
> Ces éléments sont marqués **`[ATTESTÉ 7.1]`** — donc à **re-vérifier sur le SDP 8.0**, la
> grammaire pouvant différer entre 7.1 et 8.0.
> **Correction apportée** : le modificateur s'écrit **`no-inherit`** (avec tiret), et non
> `noinherit` comme le laissait penser la lecture indirecte de la doc.
> **Toujours non attesté** : la forme complète de la règle `allow <src> <dst>:<ability>{…}`.

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

## 3. Règles d'accès

Formes **attestées sur un système QNX réel** (`preuve/qnx71/syntaxe-attenstee.txt`, chaînes de
format de l'outil officiel `/system/xbin/secpol`) — le `%s` marque un emplacement d'argument :

```
derive_type %s %s %s;              # 3 arguments, terminé par « ; »   [ATTESTÉ 7.1]
allow_link   %s %s%s;              # 2 arguments + modificateurs, « ; »  [ATTESTÉ 7.1]
allow_attach %s %s%s;              # 2 arguments + modificateurs, « ; »  [ATTESTÉ 7.1]
allow_attach %s %s%s %s;           # variante à 3 arguments           [ATTESTÉ 7.1]
```

**Conclusion structurelle majeure** : les instructions du langage se terminent par un
**point-virgule `;`** `[ATTESTÉ 7.1]`. C'était le principal point `À VÉRIFIER` du terminail.

**Modificateurs réels** (les 8 chaînes littérales rendues par l'outil) `[ATTESTÉ 7.1]` :

```
allow   deny   inherit   no-inherit   nonroot   root   lock   subrange
```

> ⚠️ **Écarts avec la lecture indirecte de la documentation** (`qnx-inventaire-officiel.md`) :
> la doc suggérait `noinherit` et `unlock` ; le binaire QNX réel donne **`no-inherit`** (avec
> tiret) et **`lock`** (pas `unlock`). De plus `gain_priv` n'apparaît **pas** comme
> modificateur dans le visualiseur — il est probablement rendu autrement (à trancher sur le
> SDP 8.0). **C'est exactement pourquoi il ne fallait rien inventer.**

Formes **non attestées** (relevées dans la doc, terminaux exacts `À VÉRIFIER`) :

```
allow <src> <dst>:<ability>{ <plages> }        # À VÉRIFIER — absent des chaînes de l'outil
allow <src> <dst>:channel connect              # À VÉRIFIER (le visualiseur rend « %s can connect to: »)
default_spawn_type …                           # À VÉRIFIER (rendu sous « spawn types: » et « %s -> %s »)
mount_point <uid>:<gid> <perms/ACL> <chemins>  # À VÉRIFIER
```

- **Wildcards** : `*` et `...` dans les chemins (`allow_attach`) `[SOURCÉ doc]` ; la chaîne
  ` # pattern` et `/...` apparaissent dans l'outil `[ATTESTÉ 7.1]`.
- **Plages** : le modificateur `subrange` est attesté, et le visualiseur rend les plages sous
  la forme ` %lu-%lu` — donc une plage s'écrit **`<début>-<fin>`** `[ATTESTÉ 7.1]`.
- `gain_priv` : acquis via le graphe `settypeid` ; la doc prescrit l'essai-erreur pour savoir
  quel type apporte quel privilège (trou S8) `[SOURCÉ doc]`.

## 3bis. Le blob compilé `secpol.bin` `[ATTESTÉ 7.1]`

L'outil officiel valide un blob typé. Types de blobs observés :
`SECPOL_BLOB_TYPE_MAP`, `SECPOL_BLOB_PMA_RULES`, `SECPOL_BLOB_PMR`, `SECPOL_BLOB_QNET`,
`SECPOL_BLOB_DERIVE`, `SECPOL_BLOB_RANGE`, `SECPOL_BLOB_CUSTOM_INDEX`.

Tableaux internes : `type array`, `rules array`, `domains array`, `range array`,
`permission array`, `ability data array`, `channel types array`, `dynamic ability names array`,
`dynamic ability permission array`, `path table`, `string table`, `name table`, `mapping array`,
`index array`, `sorted id array`, `id to name array`.

**Contrôles d'intégrité faits par l'outil officiel** (donc *après* compilation) :

```
type array at index %d references mapping outside mapping array
rule array at index %d references path outside path string table
domains array at index %d references rule outside rule array
invalid type %u in type array
invalid type %u in channel type array
duplicate type at index %d
%s extends beyond blob · %s size too large · %s has too many elements
```

> **Opportunité produit directe** : ces vérifications n'existent qu'**après** compilation.
> Tamis peut les faire **à froid**, sur le texte, avant tout déploiement — c'est cohérent avec
> le trou S3 (« aucune analyse statique avant exécution »).

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
| Chemin de la politique compilée `/proc/boot/secpol.bin` | `[ATTESTÉ 7.1]` |
| Existence de `type`, `attribute`, `class`, `range`, `default_rules` | `[SOURCÉ]` |
| Existence de `allow`, `allow_attach`, `allow_link`, `default_spawn_type`, `derive_type`, `mount_point` | `[SOURCÉ]` |
| **Terminateur `;` des instructions** | `[ATTESTÉ 7.1]` |
| **Forme de `derive_type` (3 args), `allow_link` (2 args + modif.), `allow_attach` (2 ou 3 args + modif.)** | `[ATTESTÉ 7.1]` |
| **8 modificateurs réels** : `allow`, `deny`, `inherit`, `no-inherit`, `nonroot`, `root`, `lock`, `subrange` | `[ATTESTÉ 7.1]` — corrige `noinherit`/`unlock` |
| **Écriture d'une plage `<début>-<fin>`** | `[ATTESTÉ 7.1]` |
| **66 noms d'abilities** | `[ATTESTÉ 7.1]` — voir `preuve/qnx71/abilities-binaires.txt` |
| **Structure du blob compilé** (`SECPOL_BLOB_*`, 16 tableaux, contrôles d'intégrité) | `[ATTESTÉ 7.1]` |
| Wildcards `*` et `...` | `[SOURCÉ]` + `# pattern`, `/...` `[ATTESTÉ 7.1]` |
| Fusion des sous-plages contiguës par `secpolcompile` | `[SOURCÉ]` |
| **Forme complète de `allow <src> <dst>:<ability>{…}`** | **À VÉRIFIER** — absente des chaînes de l'outil |
| Forme exacte de `range` / fichier `ability:range_name:syspage:/…` | **À VÉRIFIER** |
| Forme exacte de `default_spawn_type`, `mount_point`, `attribute`, `class` | **À VÉRIFIER** |
| Placement de `gain_priv` (non vu comme modificateur) | **À VÉRIFIER** |
| Grammaire EBNF complète et non ambiguë | **À VÉRIFIER** — bloque le parseur |
| **Re-vérification sur QNX 8.0** de tout ce qui est `[ATTESTÉ 7.1]` | **À FAIRE** dès le SDP |

## 8. Prochaine étape (E1 à trancher)

1. Obtenir le SDP 8.0 (licence gratuite myQNX — action humaine) **ou** un accès hors-ligne
   à la doc (`secpol_language.html` et pages liées).
2. Lire **page à page** et remplacer chaque `À VÉRIFIER` par la production exacte + la page.
3. **Geler** ce fichier, puis seulement écrire `core/src/lex.c` + `ast.c` (lot L1).
4. Croiser avec une politique d'exemple publique (essai E3) si disponible.

Jusqu'au gel : Tamis applique le **mode sous-ensemble strict** (voir `docs/DECISIONS.md`) —
il refuse d'analyser ce qu'il ne sait pas parser, et le dit (`TAMIS_PARSE_ERROR` avec
position et cause), **jamais** d'analyse approximative.
