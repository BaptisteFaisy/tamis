<!-- SPDX-License-Identifier: MIT -->
# Affirmations sur QNX 8.0 — traçabilité et niveau de confiance

**Date : 12/11/2026.** *Règle d'or : toute affirmation sur QNX 8.0 a une source ; ce qui
n'en a pas est marqué DÉDUIT et ne doit pas être présenté aux juges comme un fait.*

Trois niveaux : **VÉRIFIÉ** (preuve rejouable dans ce dépôt) · **ATTESTÉ 7.1** (établi sur un
vrai QNX Neutrino 7.1, à re-vérifier sur 8.0) · **DÉDUIT** (issu de la documentation, à confirmer).

## Tableau principal

| # | Affirmation | Niveau | Source | Où la vérifier |
|---|---|---|---|---|
| 1 | `secpolcompile`, `secpolgenerate`, `secpolpush` sont des **utilitaires hôtes** | VÉRIFIÉ | compilé une politique sur le r630 ; absents de l'invité QNX 7.1 | `preuve/qnx8/usage.txt`, `preuve/qnx71/session-qnx71.txt`, `banc/verifie.sh` |
| 2 | `secpolcompile` **accepte** une politique de notre corpus | VÉRIFIÉ | `0 errors, 2 warnings` | `preuve/qnx8/compilation.txt` |
| 3 | `secpolcompile` **refuse** un type non déclaré | VÉRIFIÉ | `undefined type 'app_tiers'` | `preuve/qnx8/refus.txt` |
| 4 | la politique compilée se lit depuis `/proc/boot/secpol.bin` | ATTESTÉ 7.1 | message réel de l'outil `secpol` | `preuve/qnx71/session-qnx71.txt` |
| 5 | `libsecpol.so.1` (21 139 o) est dans `/proc/boot` de l'invité | ATTESTÉ 7.1 | `ls /proc/boot` | `preuve/qnx71/session-qnx71.txt` |
| 6 | les instructions du langage se terminent par `;` | VÉRIFIÉ | productions de la doc 8.0 + acceptation du compilateur | `docs/GRAMMAR-auto.md`, `docs/GRAMMAR.md` §1-2 |
| 7 | `allow <src> <dst>:ability { … };` — la cible des abilities est `self` | VÉRIFIÉ | erreur réelle du compilateur : « cannot target the abilities of another type » | `preuve/qnx8/`, `docs/GRAMMAR.md` §2 |
| 8 | le graphe de privilège s'exprime par les arguments `settypeid:`/`gain_priv:` | VÉRIFIÉ | formes dans la doc 8.0 | `docs/GRAMMAR-auto.md` |
| 9 | les modificateurs 8.0 sont `nonroot`, `unlock`, `noinherit`, `inherit`, `deny`, `allow`, `subrange`, `root` | VÉRIFIÉ | `doc_recherche.py` sur la doc 8.0 : `unlock` et `noinherit` présents, `no-inherit` absent | `docs/GRAMMAR.md` §4 |
| 10 | les commentaires commencent par `#` (et non `<!-- -->`) | VÉRIFIÉ | `invalid token '<!--'` du compilateur | `preuve/qnx8/` |
| 11 | les plages s'écrivent `0x…-0x…`, `4-6`, `23`, `96-` (borne ouverte) | VÉRIFIÉ | doc 8.0 | `docs/GRAMMAR.md` §3 |
| 12 | une ability peut référencer une **plage nommée** (`mem_phys: gpu_mem`) | VÉRIFIÉ | doc 8.0 | `docs/GRAMMAR.md` §3 |
| 13 | les classes custom s'écrivent `class <nom> { perm … };` | VÉRIFIÉ | doc 8.0 | `docs/GRAMMAR.md` §1 |
| 14 | 66 noms d'abilities (SPAWN_SETUID, SETTYPEID, PATH_TRUST, MAC_POLICY, …) | ATTESTÉ 7.1 | binaire `secpolmonitor` de QNX 7.1 | `preuve/qnx71/abilities-binaires.txt` |
| 15 | le compilateur **fusionne les sous-plages contiguës** | DÉDUIT | doc QNX (`abilities.html`) | à vérifier par un test dédié dès le parseur |
| 16 | `default_spawn_type` n'est **jamais généré** par `secpolgenerate` | DÉDUIT | `secpol_language.html` (trou S7 du cahier) | à confirmer |
| 17 | la revue des règles inutiles est **manuelle** | DÉDUIT | `review_policies.html` (trou S1) | à confirmer |
| 18 | **aucune analyse statique avant déploiement** | DÉDUIT | `using_security_policies.html` (trou S3) | à confirmer |
| 19 | la politique générée n'est **pas utilisable telle quelle** | DÉDUIT | `security_policy_maintenance.html` (trou S2) | à confirmer |
| 20 | la fusion, la vérification du typage, les plages nommées non validables à froid | DÉDUIT | `security_policy_maintenance.html`, `named_range_file.html` (S9/S10/S13) | à confirmer |
| 21 | l'élévation de privilèges se pratique par **essai-erreur** | DÉDUIT | `secpol_language.html`/`troubleshooting.html` (trou S8) | à confirmer |
| 22 | les objets typés sont **limités aux canaux** | DÉDUIT | `using_security_policy_types.html` (trou S15) | à confirmer |
| 23 | le libellé exact de `secpolmonitor` : « Nothing to do, none of the options -a, -p, -q or -u were specified » | ATTESTÉ 7.1 | message réel | `preuve/qnx71/session-qnx71.txt` |
| 24 | `secpolcompile -e` produit un **fichier d'événements secpolgenerate** | VÉRIFIÉ | `usage.txt` ; l'option existe dans le binaire installé | `preuve/qnx8/usage.txt` |

## Écarts 7.1 → 8.0 (à arbitrer)

| Sujet | 7.1 (binaire) | 8.0 (doc) | Conséquence |
|---|---|---|---|
| Modificateur | `no-inherit` (tiret), `lock` | `noinherit`, `unlock` | le parseur 8.0 doit accepter la forme **8.0** ; `docs/GRAMMAR.md` §4 |
| Ciblage des abilities | (non testé) | `self` obligatoire | les edges `settypeid:`/`gain_priv:` portent le graphe |

## Ce que le dossier Devpost ne doit PAS affirmer

| ❌ Proscrit | ✅ Honnête |
|---|---|
| « analyse les politiques QNX 8.0 » | « analyse un modèle de politique conforme à la grammaire QNX 8.0 relevée dans la doc officielle et validée par `secpolcompile` » |
| « détecte les failles de QNX » | « détecte les chemins de privilège dans le modèle de politique » |
| « validé contre `secpolcompile` » | « le compilateur officiel accepte notre politique de référence ; le différentiel de parseur s'active dès que le parseur `secpol` est écrit » |
| « 100 % des politiques couvertes » | « le périmètre de complétude est publié, avec ses angles morts bornés » |
| « remplace `secpolgenerate` » | « outille la partie que QNX laisse à la main : relire, fusionner, justifier » |

## Relecture par un pair

Chaque ligne VÉRIFIÉ pointe vers un chemin de fichier rejouable. La relecture consiste à
ouvrir chaque source et à cocher. Les lignes DÉDUIT sont fermées automatiquement à terme :
`tools/extract_grammar.py` (doc 8.0) et `banc/verifie.sh` (cible 7.1) relisent les sources.