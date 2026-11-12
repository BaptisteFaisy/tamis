<!-- SPDX-License-Identifier: MIT -->
# Brouillon de grammaire — extraction automatique depuis les docs du SDP

> **GÉNÉRÉ automatiquement** par `tools/extract_grammar.py` depuis les pages HTML
> installées avec le SDP. Chaque production est à **relire** avant de figer
> `docs/GRAMMAR.md` : l'extraction est littérale, elle ne garantit pas l'exactitude
> sémantique (les docs contiennent aussi des exemples et des fragments de code).

Pages analysées : **1** · productions relevées : **10**

## secpol_language.html

| Production | Source |
|---|---|
| `type <type_name> ;` | secpol_language.html |
| `attribute <name> { <type_list> } ;` | secpol_language.html |
| `range <name> ;` | secpol_language.html |
| `allow <src> <dst>:<ability> { <ability_list> } ;` | secpol_language.html |
| `allow <src> <dst>:channel connect ;` | secpol_language.html |
| `allow_attach <type> <path> [ <ftype> : ] ;` | secpol_language.html |
| `allow_link <type> <path> ;` | secpol_language.html |
| `default_spawn_type <type> ;` | secpol_language.html |
| `derive_type <type> <base> ;` | secpol_language.html |
| `mount_point <uid>:<gid> <perms> ;` | secpol_language.html |

## Vocabulaire relevé

- Mots-clés vus : `allow, allow_attach, allow_link, attribute, default_spawn_type, derive_type, mount_point, range, type`
- Modificateurs vus : `allow, deny, inherit, lock, no-inherit, nonroot, root, subrange`

À comparer avec les modificateurs attestés sur QNX 7.1 (`preuve/qnx71/syntaxe-attenstee.txt`) : toute divergence est un signal fort.
