<!-- SPDX-License-Identifier: MIT -->
# Preuve — banc QNX réel (QNX Neutrino 7.1.0)

**Capturé le 12 novembre 2026** sur un vrai système QNX démarré en QEMU sur le poste de
développement. Ce dossier existe pour une raison : la documentation officielle QNX 8.0 en
ligne renvoyait **404** pendant tout le développement, et le **SDP 8.0 n'est pas installé**
(licence gratuite, demande en attente). Plutôt que d'inventer la syntaxe, on est allé la
chercher sur un système QNX réel.

> **Règle respectée** : l'image QNX n'est **pas redistribuée** (la licence communautaire ne le
> permet probablement pas). On publie **l'empreinte** et la **procédure de reproduction**, plus
> des **faits** (noms d'abilities, formes syntaxiques, chemins, messages d'outils) — jamais
> l'image elle-même.

## Provenance

| Élément | Valeur |
|---|---|
| Système invité | QNX Neutrino **7.1.0** (2022/07/12-18:14:39EDT), x86pc, x86_64 |
| Image | communautaire `autosarai/qemu_qnx7_1`, disque brut 3 partitions QNX (b1/b2/b3) |
| Taille | 6 500 959 232 octets (6,05 Gio) |
| **SHA-256 avant démarrage** | `03180f99d38af98066d459427ebbe41859b10de6eb9f10763c02c5b4a50c3d43` |
| **SHA-256 après la session** | `30c0a7bfa896ea7bcb5b3522d697c4238ca779201464ad5d6b32f44d1e3d5d5e` |
| **SHA-256 image amont intacte** | `671a12dbfaa2aabe8e55713abe2734ab7b2b01b581e22021feeef3dafcab5e7f` |
| Hyperviseur | QEMU 8.2.2 (hôte Linux), 1 Go RAM, 2 vCPU, console VGA + SSH |
| Noyau observé | `proc/boot/procnto-smp-instr` (pid 1) |

> **Note d'honnêteté sur les empreintes** : un disque d'invité **change à chaque démarrage**
> (l'invité y écrit : journaux, horodatages, état du système de fichiers). Une empreinte
> identifie donc un **état**, pas une version. Les trois valeurs ci-dessus sont données pour
> ça : avant la session de capture, après, et celle de l'image amont jamais démarrée.
> **Toute affirmation de ce dossier a été capturée pendant la session `03180f99… → 30c0a7bf…`.**

## Reproduction

```sh
# hôte : démarrer l'invité QNX (image non fournie, voir empreinte ci-dessus)
qemu-system-x86_64 -drive format=raw,file=qnx71-fs.img -m 1024 -smp 2 \
  -netdev user,id=n0,net=192.168.100.0/24,hostfwd=tcp:127.0.0.1:2225-192.168.100.10:22 \
  -device vmxnet3,netdev=n0 -display none -serial file:serial.log \
  -monitor unix:mon.sock,server,nowait
# puis, dans l'invité (sshd de l'image communautaire) :
ssh -p 12225 root@127.0.0.1 'uname -a; ls /proc/boot | grep -i secpol; secpol; secpolmonitor'
```

Les commandes réellement jouées et leur sortie brute : **`session-qnx71.txt`**.

## Ce que cette preuve établit

| Fait | Fichier | Portée |
|---|---|---|
| **66 noms d'abilities** tels que le binaire QNX les connaît (SPAWN_SETUID, SETTYPEID, PATH_TRUST, MAC_POLICY, CHANNEL_CONNECT, ABLE_PRIV, XPROCESS_MEM_READ, SANDBOX, QVM, RCONSTRAINT, APS_ROOT, V86, QNET, KEYDATA…) | `abilities-binaires.txt` | corrige et complète `politique/abilities.tsv` (listé d'après la doc) |
| **Formes syntaxiques réelles** : `derive_type %s %s %s;`, `allow_link %s %s%s;%s`, `allow_attach %s %s%s;%s`, `allow_attach %s %s%s %s;%s` | `syntaxe-attenstee.txt` | les instructions se terminent par **`;`** ; `allow_attach` a une forme à 2 et une à 3 arguments |
| **Modificateurs réels** : `allow`, `deny`, `inherit`, `no-inherit`, `nonroot`, `root`, `lock`, `subrange` | `syntaxe-attenstee.txt` | **corrige** `noinherit` → **`no-inherit`** (avec tiret) |
| **Chemin de la politique compilée** : `/proc/boot/secpol.bin` | `session-qnx71.txt` | confirme l'insertion dans l'IFS |
| **Outils présents sur la cible** : `/system/xbin/secpol` (visualiseur), `/system/xbin/secpolmonitor`, `libsecpol.so.1` (21 139 octets) | `session-qnx71.txt` | le visualiseur lit le blob compilé |
| **Outils absents de la cible** : `secpolcompile`, `secpolgenerate`, `secpolpush` | `session-qnx71.txt` | confirme que ce sont des **utilitaires hôtes** (SDP) — d'où le différentiel possible sans cible |
| **Structure du blob compilé** : `SECPOL_BLOB_TYPE_MAP`, `SECPOL_BLOB_PMA_RULES`, `SECPOL_BLOB_PMR`, `SECPOL_BLOB_QNET`, `SECPOL_BLOB_DERIVE`, `SECPOL_BLOB_RANGE`, `SECPOL_BLOB_CUSTOM_INDEX` ; tableaux `type`, `rules`, `domains`, `range`, `permission`, `path table`, `string table`, `mapping`, `channel types`, `dynamic ability names/permission`, `sorted id`, `id to name`, `index` | `syntaxe-attenstee.txt` | carte du format binaire — utile pour `oracle/` (TAM-12) |
| **Contrôles d'intégrité du blob** faits par l'outil officiel : « type array at index %d references mapping outside mapping array », « rule array at index %d references path outside path string table », « domains array at index %d references rule outside rule array », « invalid type %u in type array », « duplicate type at index %d » | `syntaxe-attenstee.txt` | ce sont des vérifications **après** compilation — Tamis peut les faire **avant**, à froid |
| **Comportement réel sans politique** : `secpol` → `/proc/boot/secpol.bin: No such file or directory` (rc 1) ; `secpolmonitor` → `Nothing to do, none of the options -a, -p, -q or -u were specified` (rc 1) | `session-qnx71.txt` | les deux outils exigent une politique compilée ; options réelles de `secpolmonitor` : `acf:i:no:pqrsS:uvV` |
| `/proc/boot/ability -> on` | `session-qnx71.txt` | marqueur d'activation des abilities dans l'IFS |

## Ce que cette preuve n'établit PAS

- **C'est QNX 7.1, le projet vise QNX 8.0.** La grammaire peut différer entre les deux
  versions : tout ce qui est attesté ici reste à **re-vérifier sur le SDP 8.0** dès la licence
  obtenue. Les noms d'abilities et les formes de base ont peu de chances d'avoir changé, mais
  ce n'est pas une raison pour l'affirmer.
- **La syntaxe complète de la règle `allow`** (`allow <src> <dst>:<ability>{…}`) n'apparaît
  **pas** dans les chaînes de l'outil : le visualiseur rend les connexions sous forme résumée
  (`%s can connect to:`). Elle reste **À VÉRIFIER**.
- **Aucune politique n'était chargée** dans cet invité (`/proc/boot/secpol.bin` absent) et
  `secpolcompile` est un outil hôte absent de la cible : **aucun test de compilation ni
  d'application réelle** n'a pu être fait. Le différentiel Tamis ↔ `secpolcompile` (TAM-12)
  attend toujours le SDP.

## Note de licence

Ne sont reproduits ici que des **faits** (noms d'identifiants, chemins, formes syntaxiques) et
de **courtes chaînes de format**, à des fins d'interopérabilité et de documentation technique.
Aucun binaire QNX, aucune image, aucun extrait substantiel de documentation QNX n'est
redistribué. L'image utilisée provient d'un dépôt communautaire tiers et reste sur le poste de
développement ; seule son empreinte est publiée, pour permettre la vérification.
