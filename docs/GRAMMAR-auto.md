<!-- SPDX-License-Identifier: MIT -->
# Brouillon de grammaire — extraction automatique depuis les docs du SDP

> **GÉNÉRÉ automatiquement** par `tools/extract_grammar.py` depuis les pages HTML
> installées avec le SDP. Chaque production est à **relire** avant de figer
> `docs/GRAMMAR.md` : l'extraction est littérale, elle ne garantit pas l'exactitude
> sémantique (les docs contiennent aussi des exemples et des fragments de code).

Pages analysées : **1** · productions relevées : **10**

## secpol_language.html

```
type <type_name> ;
```

```
attribute <name> { <type_list> } ;
```

```
range <name> ;
```

```
allow <src> <dst>:<ability> { <ability_list> } ;
```

```
allow <src> <dst>:channel connect ;
```

```
allow_attach <type> <path> [ <ftype> : ] ;
```

```
allow_link <type> <path> ;
```

```
default_spawn_type <type> ;
```

```
derive_type <type> <base> ;
```

```
mount_point <uid>:<gid> <perms> ;
```

## Vocabulaire relevé

- Mots-clés vus : `allow, allow_attach, allow_link, attribute, default_spawn_type, derive_type, mount_point, range, type`
- Modificateurs vus : ``
- Modificateurs de référence (QNX 7.1 réel) : `allow, deny, inherit, no-inherit, nonroot, root, lock, subrange`
- Abilities vues (0) : ``

À comparer avec les modificateurs attestés sur QNX 7.1 (`preuve/qnx71/syntaxe-attenstee.txt`) : toute divergence est un signal fort.
