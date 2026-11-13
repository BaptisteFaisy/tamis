<!-- SPDX-License-Identifier: MIT -->
# ORACLE SDP — table de vérité `secpolcompile` (QNX 8.0)

**Tamis / TAM-12 — différentiel oracle.** Capturé le **12/11/2026** sur le serveur
**r630**, SDP QNX 8.0 installé dans `~/qnx800`
(binaire : `~/qnx800/host/linux/x86_64/usr/bin/secpolcompile`).

Ce document est **la référence** contre laquelle le parseur de Tamis devra être
confronté : pour chaque cas du corpus `fixtures/secpol-cas/`, ce que le
compilateur officiel accepte ou refuse, et **le message exact** qu'il produit.

La trace brute, rejouable, est dans `preuve/qnx8/oracle-table.txt`.

## 0. Mode d'emploi et règles de lecture

```sh
ssh r630
. ~/qnx800/qnxsdp-env.sh
cd /tmp/tamis-oracle-cas
secpolcompile -o <cas>.bin <cas>.secpol
```

(page de doc : `compile.html` — « Compiling the security policy »)

Quatre faits **observés** sur les 42 cas, qui pilotent l'implémentation du
différentiel :

1. `secpolcompile` écrit **tout sur stdout** ; les erreurs sont au format
   `<fichier>:<ligne>:<colonne>: error: <message>`.
2. **La ligne `N errors, M warnings` n'est affichée que s'il y a au moins un
   message.** Sur une politique parfaite, `secpolcompile` n'affiche **rien du
   tout** (ni `0 errors, 0 warnings`). Un oracle naïf qui attend la ligne de
   compte se plante donc sur les cas valides.
3. **Le code de sortie vaut 1 dès qu'il y a au moins une erreur**, 0 sinon.
   Contraryment à ce que l'on croyait jusqu'ici, l'outil *ne* sort pas 0 en cas
   d'erreur : le code de sortie est l'oracle le plus fiable, la ligne de compte
   n'est qu'un confort.
4. La **colonne** est en base 1 ; la valeur `0` signifie « pas de colonne »
   (erreur de fin de fichier : la ligne vaut alors le numéro de ligne suivant
   la dernière).

> Les pages de doc citées ci-dessous sont celles installées avec le SDP :
> `~/qnx800/target/qnx/usr/help/eclipse/plugins/com.qnx.doc.security.system/topic/manual/<page>.html`.
> La référence du langage est **`secpol_language.html`** (« Security policy
> language ») ; la liste des abilities est **`abilities.html`** (« Abilities ») ;
> les plages nommées : **`named_range_file.html`**.

## 1. Table de vérité

| Cas | Fichier | Verdict `secpolcompile` | Message exact | Règle QNX déduite |
|---|---|---|---|---|
| V01 | `valide-01.secpol` | **0 erreur** (aucune sortie) | — | `type X;` puis `allow X self:ability { … };` — `secpol_language.html` §« Granting procmgr abilities » |
| V02 | `valide-02.secpol` | **0 erreur** (aucune sortie) | — | ability avec arguments numériques (`mem_phys`, `setuid`) — `secpol_language.html` §« Ability ranges » |
| V03 | `valide-03.secpol` | **0 erreur** (aucune sortie) | — | plages mixtes + borne ouverte `96-` (fin = plus grande valeur) — `secpol_language.html` §« Ability ranges » |
| V04 | `valide-04.secpol` | **0 erreur** (aucune sortie) | — | plages nommées : `range X;` puis `mem_phys: X` — `secpol_language.html` §« Named ranges » + `named_range_file.html` |
| V05 | `valide-05.secpol` | **0 erreur** (aucune sortie) | — | classe custom `class file { … };` + `allow p f : file read;` — `secpol_language.html` §« Using custom classes and actions » |
| V06 | `valide-06.secpol` | **0 erreur** (aucune sortie) | — | canal : `allow src dst:channel connect;` — `secpol_language.html` §« Rules for connecting to channels » |
| V07 | `valide-07.secpol` | **0 erreur** (aucune sortie) | — | `default_spawn_type t1 t2;` — `secpol_language.html` §« Forcing a type change when spawning » |
| V08 | `valide-08.secpol` | **0 erreur** (aucune sortie) | — | commentaires `#` jusqu'à la fin de la ligne — `secpol_language.html` §1 |
| V09 | `valide-09.secpol` | **0 erreur** (aucune sortie) | — | modificateurs **`nonroot`**, **`unlock`**, **`noinherit`** — `secpol_language.html` §« Granting abilities to non-root processes », §« Unlocking abilities », §« Changing whether abilities are inherited » |
| V10 | `valide-10.secpol` | **0 erreur** (aucune sortie) | — | ability dynamique déclarée `ability network/bind/privport;` — `secpol_language.html` §« Custom abilities » |
| V11 | `valide-11.secpol` | **0 erreur** (aucune sortie) | — | `attribute secure;` + `type X, secure;` — `secpol_language.html` §« Using an attribute to group types » |
| V12 | `valide-12.secpol` | **0 erreur** (aucune sortie) | — | `derive_type` (forme simple et forme ensembles) — `secpol_language.html` §« Derived types » |
| V13 | `valide-13.secpol` | **0 erreur** (aucune sortie) | — | `allow_attach` / `allow_link`, préfixes `mount:` et `mount+:`, jokers `*` et `…` — `secpol_language.html` §« Allow a process to attach one of its channels to a path » |
| V14 | `valide-14.secpol` | **0 erreur** (aucune sortie) | — | `mount_point uid:gid perms { … } ;` — `secpol_language.html` §« Setting ownership and permissions for mount points » |
| V15 | `valide-15.secpol` | **0 erreur** (aucune sortie) | — | ensembles `{ t1 t2 } { t3 t4 }:channel connect;` — `secpol_language.html` §« Rules for sets of types, targets, and classes » |
| V16 | `valide-16.secpol` | **0 erreur** (aucune sortie) | — | transition cohérente `settypeid:` + `gain_priv:` (0 warning) — `secpol_language.html` §« Specifying which privileges can be acquired by switching type » |
| V17 | `valide-17.secpol` | **0 erreur, 2 warnings** | `warning: type app_tiers has unneeded gain_priv abilities:` ⏎ `    file:read:file1_t` ⏎ `warning: type diag has unneeded gain_priv abilities:` ⏎ `    pathspace` | un `gain_priv` inutile est un **warning**, pas une erreur — même section que V16 |
| I01 | `invalide-01.secpol` | **1 erreur** | `invalide-01.secpol:2:7: error: undefined type 'app_tiers'` | tout type doit être déclaré avant d'être utilisé |
| I02 | `invalide-02.secpol` | **1 erreur** | `invalide-02.secpol:3:6: error: type 'app_tiers' has already been declared` | un nom de type est unique dans la politique |
| I03 | `invalide-03.secpol` | **1 erreur** | `invalide-03.secpol:5:5: error: undefined ability unknown_ability` | ability absente du vocabulaire QNX 8.0 |
| I04 | `invalide-04.secpol` | **1 erreur** | `invalide-04.secpol:6:0: error: syntax error` | accolade fermante manquante → erreur rapportée à la **fin de fichier** |
| I05 | `invalide-05.secpol` | **1 erreur** | `invalide-05.secpol:4:1: error: syntax error` | `;` manquant : toute instruction se termine par `;` — `secpol_language.html` §1 |
| I06 | `invalide-06.secpol` | **1 erreur** | `invalide-06.secpol:7:0: error: syntax error` | `;` manquant après la fermeture `}` du bloc d'abilities |
| I07 | `invalide-07.secpol` | **1 erreur** | `invalide-07.secpol:7:1: error: type 'app_tiers' cannot target the abilities of another type ('other')` | la cible d'un bloc d'abilities est **`self`** — `secpol_language.html` §« The self type » |
| I08 | `invalide-08.secpol` | **1 erreur** | `invalide-08.secpol:5:5: error: bad range '4-6-9' for ability 'setuid'` | une plage = une seule paire début-fin ; le tiret ne peut pas se répéter |
| I09 | `invalide-09.secpol` | **1 erreur** | `invalide-09.secpol:5:5: error: end less than start in range '4096-1024' for ability 'mem_phys'` | le début d'une plage ne peut pas dépasser sa fin |
| I10 | `invalide-10.secpol` | **1 erreur** | `invalide-10.secpol:6:1: error: syntax error` | `ability:` suivi d'aucun argument → erreur de syntaxe générique |
| I11 | `invalide-11.secpol` | **1 erreur** | `invalide-11.secpol:5:5: error: ability 'able_create' does not support sub-ranges` | certaines abilities n'acceptent **pas** de sous-plage — `abilities.html` §« Ability ranges » |
| I12 | `invalide-12.secpol` | **1 erreur** | `invalide-12.secpol:5:21: error: undefined class 'file'` | la classe custom doit être déclarée avant d'être utilisée |
| I13 | `invalide-13.secpol` | **1 erreur** | `invalide-13.secpol:7:26: error: undefined permission 'connect' for file class` | la permission demandée doit appartenir à la classe — `secpol_language.html` §« Using custom classes and actions » |
| I14 | `invalide-14.secpol` | **1 erreur** | `invalide-14.secpol:3:7: error: class 'file' has already been declared` | un nom de classe est unique |
| I15 | `invalide-15.secpol` | **1 erreur** | `invalide-15.secpol:2:25: error: permission 'read' has already been declared for class 'file'` | une permission est unique dans sa classe |
| I16 | `invalide-16.secpol` | **1 erreur** | `invalide-16.secpol:5:5: error: undefined type 'undefined_type' for range for ability 'settypeid'` | l'argument de `settypeid:` est un **nom de type** — `secpol_language.html` §« Ability ranges » |
| I17 | `invalide-17.secpol` | **1 erreur** | `invalide-17.secpol:5:5: error: named range 'abc' has not been declared for ability 'mem_phys'` | une plage nommée doit être déclarée par `range X;` — `named_range_file.html` |
| I18 | `invalide-18.secpol` | **1 erreur** | `invalide-18.secpol:5:30: error: ability 'network/bind/privport' has already been declared` | une ability dynamique ne se déclare qu'une fois |
| I19 | `invalide-19.secpol` | **1 erreur** | `invalide-19.secpol:2:17: error: 'secure' has not been declared as an attribute` | un attribut doit être déclaré par `attribute X;` |
| I20 | `invalide-20.secpol` | **1 erreur** | `invalide-20.secpol:5:5: error: undefined ability no-inherit` | `no-inherit` est la forme **7.1** ; en 8.0 c'est `noinherit` |
| I21 | `invalide-21.secpol` | **1 erreur** | `invalide-21.secpol:4:30: error: undefined type 'nonexistent'` | `default_spawn_type` vise un type qui doit exister |
| I22 | `invalide-22.secpol` | **2 erreurs** (1 faute) | `invalide-22.secpol:2:6: error: invalid token 'app-tiers'` ⏎ `invalide-22.secpol:2:6: error: syntax error` | pas de tiret dans un nom de type — `secpol_language.html` §« Defining types » |
| I23 | `invalide-23.secpol` | **1 erreur** | `invalide-23.secpol:4:16: error: syntax error` | une règle `allow` exige une classe **et** une permission |
| I24 | `invalide-24.secpol` | **1 erreur** | `invalide-24.secpol:6:37: error: channel for type 'random_t' cannot be relabeled to 'socket_t' as the rule for the path '/dev/random' did not relabel it` | le relabel de canal doit être cohérent entre les règles d'un même type |
| I25 | `invalide-25.secpol` | **2 erreurs** (1 faute) | `invalide-25.secpol:5:14: error: undefined ability ` ⏎ `invalide-25.secpol:5:16: error: undefined ability 6` | pas d'espace autour du `-` d'une plage (`setuid:4 - 6` est refusé) |

> 42 cas : **17 valides** (V01→V17) et **25 invalides** (I01→I25).
> 23 des 25 cas invalides produisent **exactement une** erreur ; I22 et I25
> produisent deux messages pour **une seule faute** (le second message est la
> conséquence du premier). Chaque invalide ne contient qu'**une** construction
> fautive, c'est ce qui rend le message net.

## 2. Ce que le compilateur refuse, et pourquoi

Regroupement par **catégorie de refus**. Chaque catégorie donne le message
exact, et la construction fautive.

> Dans les tableaux ci-dessous, un message **sans nom de fichier** vient d'une
> **sonde hors corpus** (même chaîne de compilation, fichier non conservé) :
> le message est vérifié, mais il n'a pas de fixture associée.

### 2.1 Type non déclaré — `undefined type 'X'`

| Où | Message exact |
|---|---|
| source d'une règle | `invalide-01.secpol:2:7: error: undefined type 'app_tiers'` |
| cible d'une règle | `error: undefined type 'secure'` (attribute non déclarée, cf. I19) |
| argument de `default_spawn_type` | `invalide-21.secpol:4:30: error: undefined type 'nonexistent'` |
| argument de `derive_type` | `error: undefined type 'nonexistent'` |
| type de canal d'un `allow_attach` | `error: undefined type 'unknown_type'` |

**Règle** : tout identifiant de type doit exister dans la politique.

### 2.2 Symbole déjà déclaré — `… has already been declared`

| Symbole | Message exact |
|---|---|
| type | `invalide-02.secpol:3:6: error: type 'app_tiers' has already been declared` |
| classe | `invalide-14.secpol:3:7: error: class 'file' has already been declared` |
| permission (dans sa classe) | `invalide-15.secpol:2:25: error: permission 'read' has already been declared for class 'file'` |
| ability dynamique | `invalide-18.secpol:5:30: error: ability 'network/bind/privport' has already been declared` |

**Règle** : types, classes, abilities dynamiques et permissions sont uniques.
Une classe re-déclarée **avec d'autres permissions** ne donne qu'**une** erreur
(c'est le cas I14) ; avec les mêmes permissions, c'est une erreur par permission
en double.

### 2.3 Ability inconnue — `undefined ability X`

| Cas | Message exact |
|---|---|
| ability statique inconnue | `invalide-03.secpol:5:5: error: undefined ability unknown_ability` |
| ability dynamique non déclarée | `error: undefined ability network/bind/privport` |
| modificateur 7.1 | `invalide-20.secpol:5:5: error: undefined ability no-inherit` |

**Règle** : le vocabulaire d'abilities est **fermé**. Une ability statique
inconnue, une ability dynamique non déclarée par `ability …;`, ou un
modificateur hors liste 8.0 produisent le **même** message
`undefined ability <token>`. C'est aussi ce qui arrive à `inherit`, `lock`,
`deny`, `root`, `subrange` en 8.0 (voir §3, écarts).

### 2.4 Erreur de syntaxe — `syntax error`

Message unique, **sans détail** : le compilateur ne dit pas ce qui manque.

| Cas | Message exact |
|---|---|
| accolade fermante manquante | `invalide-04.secpol:6:0: error: syntax error` (colonne 0 = fin de fichier) |
| `;` manquant après `type X` | `invalide-05.secpol:4:1: error: syntax error` (colonne du token suivant) |
| `;` manquant après `}` | `invalide-06.secpol:7:0: error: syntax error` (fin de fichier) |
| `ability:` sans argument | `invalide-10.secpol:6:1: error: syntax error` |
| `allow` sans classe/permission | `invalide-23.secpol:4:16: error: syntax error` |
| token invalide (tiret) | `invalide-22.secpol:2:6: error: invalid token 'app-tiers'` **puis** `error: syntax error` |

**Règle** : quatre des cinq fautes de syntaxe du corpus sont **indistinguables**
une fois le message produit. Le différentiel oracle ne peut donc pas comparer
les messages de syntaxe plus finely que « accepté / refusé » + position.

### 2.5 Cible d'ability qui n'est pas `self`

`invalide-07.secpol:7:1: error: type 'app_tiers' cannot target the abilities of another type ('other')`

**Règle** : dans `allow <src> <dst>:ability { … };`, `<dst>` doit être `self`.
Le graphe de privilège passe par les **arguments** des abilities de transition
(`settypeid:`, `gain_priv:`), pas par la cible de la règle. La forme
`allow <src> <autre_type>:channel connect;` reste, elle, autorisée (V06) : la
restriction est propre au mot-clé `ability`.

### 2.6 Plage mal formée

| Cas | Message exact |
|---|---|
| tiret en double | `invalide-08.secpol:5:5: error: bad range '4-6-9' for ability 'setuid'` |
| fin < début | `invalide-09.secpol:5:5: error: end less than start in range '4096-1024' for ability 'mem_phys'` |
| plage nommée non déclarée | `invalide-17.secpol:5:5: error: named range 'abc' has not been declared for ability 'mem_phys'` |
| argument de plage = type inexistant | `invalide-16.secpol:5:5: error: undefined type 'undefined_type' for range for ability 'settypeid'` |
| `gain_priv:` sur un nom inconnu | `error: undefined ability name 'foo' for range for ability 'gain_priv'` |
| espace autour du tiret | `invalide-25.secpol:5:14: error: undefined ability ` (vide) puis `error: undefined ability 6` |

**Règle** : une plage est `début-fin`, début ≤ fin, et le tiret est **collé**
aux bornes. Le message `undefined ability <vide>` est le signe d'un token vide
(la suite de `setuid:4 - 6` est analysée comme « ability 4 », « ability vide »,
« ability 6 »).

### 2.7 Ability_item sans argument / argument interdit

| Cas | Message exact |
|---|---|
| `settypeid:` sans argument | `invalide-10.secpol:6:1: error: syntax error` |
| `mem_phys:` sans argument | `error: syntax error` |
| argument sur une ability sans plage | `invalide-11.secpol:5:5: error: ability 'able_create' does not support sub-ranges` |
| argument sur une ability sans plage (plage nommée) | `error: ability 'able_create' does not support sub-ranges` |

**Règle** : deux messages distincts — « argument absent » → erreur de syntaxe
générique ; « argument interdit » → message dédié
`does not support sub-ranges`. Le second est **plus précis** : c'est le seul
refus qui nomme l'ability fautive.

### 2.8 Classe custom et permissions

| Cas | Message exact |
|---|---|
| classe inconnue | `invalide-12.secpol:5:21: error: undefined class 'file'` |
| permission absente de la classe | `invalide-13.secpol:7:26: error: undefined permission 'connect' for file class` |
| classe dupliquée | `invalide-14.secpol:3:7: error: class 'file' has already been declared` |
| permission dupliquée | `invalide-15.secpol:2:25: error: permission 'read' has already been declared for class 'file'` |

**Règle** : la classe et la permission sont vérifiées **séparément** ; le
message distingue « la classe n'existe pas » de « la permission n'appartient
pas à cette classe ». C'est exactement l'exemple donné par la doc QNX
(`allow ptype ftype : file connect;` → erreur, `secpol_language.html`
§« Using custom classes and actions »).

### 2.9 Attribut non déclaré

`invalide-19.secpol:2:17: error: 'secure' has not been declared as an attribute`

**Règle** : dans `type X, attr;`, l'attribut doit exister. Message **différent**
de `undefined type` — l'attribut n'est pas un type.

### 2.10 Cohérence du relabel de canal

`invalide-24.secpol:6:37: error: channel for type 'random_t' cannot be relabeled to 'socket_t' as the rule for the path '/dev/random' did not relabel it`

**Règle** : si un type a une règle `allow_attach` **avec** relabel, toutes ses
règles `allow_attach` doivent relabeler vers le même type. Refus purement
sémantique : aucun token invalide, le fichier est syntaxiquement correct.

## 3. Écarts entre mon attente (issue de la doc / de `docs/GRAMMAR.md`) et le compilateur

Ces cas **m'ont surpris** : je m'attendais à un verdict différent. Ils sont
**conservés dans le corpus** (c'est une information précieuse), et listés ici
explicitement.

1. **`secpolcompile` sort 1 en cas d'erreur.** Le brief indiquait « sort 0 même
   en cas d'erreur ». Observé : exit=1 sur chacun des 25 invalides, exit=0 sur
   les 17 valides. → le code de sortie est un oracle sûr.

2. **`0 errors, 0 warnings` n'existe pas.** Sur une politique correcte, l'outil
   n'affiche rien. Toute la logique « j'accepte si je vois une ligne
   `0 errors` » est fausse.

3. **`type 1abc;` est accepté** (0 erreur), alors que la doc QNX écrit
   *« Type names … cannot start with a number »* (`secpol_language.html`
   §« Defining types »). Le compilateur 8.0 ne l'applique pas.

4. **`type default;` est accepté** (0 erreur), alors que la doc décrit `default`
   comme un nom réservé (type ID 0).

5. **Un `range X;` re-déclaré est accepté** (0 erreur, aucun message), alors que
   les types, classes et permissions en double sont refusés.

6. **`inherit`, `lock`, `deny`, `root`, `subrange` sont refusés** en 8.0
   (`error: undefined ability <token>`), alors que `docs/GRAMMAR.md` §4 les
   liste parmi les modificateurs officiels 8.0. Seuls **`nonroot`, `unlock`,
   `noinherit`** sont des modificateurs dans le compilateur 8.0 testé.
   → `docs/GRAMMAR.md` §4 a été corrigé sur ce point (seuls `nonroot`, `unlock`, `noinherit` sont des modificateurs en 8.0).

7. **`allow` comme modificateur** dans un bloc d'abilities donne
   `error: syntax error` (et non `undefined ability allow`), contrairement aux
   autres mots rejetés. `allow` est un mot clé réservé de la grammaire.

8. **`settypeid` sans argument est accepté** (0 erreur), ainsi que
   `channel_connect` sans argument. L'ability sans son argument est donc
   tolérée par le parseur (la « plage » est simplement absente), alors qu'un
   `:` suivi de rien est une erreur de syntaxe.

9. **Le « gain_priv manquant » n'est plus une erreur.** La doc QNX écrit :
   *« An error for any privileges that the type gains by switching type that are
   not specified by gain_priv. »* Observé : aucune erreur — le compilateur 8.0
   n'émet que le warning **inverse** (`unneeded gain_priv abilities`). Une
   transition vers un type possédant `pathspace`, sans `gain_priv`, est
   acceptée sans le moindre message (sonde hors corpus).

10. **Un nom de type avec tiret produit 2 messages** (`invalid token` + `syntax
    error`) là où la doc annonce *« will generate an error »* (singulier).

11. **Les espaces autour du tiret d'une plage sont refusés**
    (`setuid:4 - 6` → `undefined ability` + `undefined ability 6`), alors que la
    doc écrit *« You can add spaces when you specify the range »*.

## 4. À confirmer (cas ambigus, hors verdict binaire)

- **`type default_rules;` + `allow default_rules self:ability { … };`** donne
  `warning: empty policy` (exit 1) **sans** ligne `N errors, M warnings`. La
  doc (`secpol_language.html` §« Apply default rules to types ») présente
  `default_rules` comme le type spécial qui distribue des privilèges par
  défaut ; en 8.0, **non déclaré** il est refusé (`undefined type
  'default_rules'`), et **déclaré** il produit un politique vide. Ce n'est ni
  « valide » ni « invalide » au sens du corpus, d'où son absence des fixtures.
  À clarifier avec une politique réelle qui en fait usage.

- **Une politique vide** (fichier vide ou ne contenant que des commentaires)
  produit `warning: empty policy` (exit 1), toujours sans ligne de compte. Le
  différentiel devra traiter ce cas (sortie non vide, code 1, aucune ligne
  `error:`).

- **La colonne dans les messages.** L'observation sur 42 cas est cohérente avec
  une colonne **en base 1** (0 = « pas de colonne », fin de fichier). Non
  confirmé par la doc elle-même (elle ne documente pas le format de sortie).

- **Ability dynamique à 1, 2 ou 4 segments** (`ability foo;`,
  `ability network/bind;`, `ability network/bind/privport/extra;`) : toutes
  acceptées sans message. La doc ne définit qu'un exemple à 3 segments
  (`network/bind/privport`). Le nombre de segments n'est donc pas validé.

## 5. Ce que cela implique pour Tamis (TAM-12)

- Le différentiel ne peut PAS se contenter de comparer des messages : sur les
  erreurs de syntaxe (I04, I05, I06, I10, I23), le message officiel est un
  simple `syntax error` sans sémantique. **L'oracle fiable est le couple
  `(accepté/refusé, position)`**, plus le message exact quand il est informatif
  (types, abilities, classes, plages, relabel de canal).
- `secpolcompile` accepte des formes que la doc interdit (§3, points 3 à 5, 8) :
  le parseur de Tamis ne doit **pas** rejeter ces formes s'il veut rester
  d'accord avec le compilateur.
- La notion de « valide » est à traiter avec les **warnings** inclus : V17 est
  accepté (0 erreur) mais avec 2 warnings. Le verdict strictement binaire
  « accepte/refuse » est porté par les **erreurs**, pas par les warnings.
