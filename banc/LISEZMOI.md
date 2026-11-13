<!-- SPDX-License-Identifier: MIT -->
# Banc QNX 8.0 — exécuter une preuve dans une vraie cible

Ce dossier décrit le banc **QNX 8.0** : un invité QEMU réel dans lequel le cœur C de Tamis
(compilé par `qcc` pour QNX 8.0) rend ses verdicts, et un service HTTP qui rejoue une
liste **fermée** d'étapes dans cet invité, en comparant la sortie à celle du poste
**octet pour octet**.

> Le banc **QNX 7.1** (image de community, `qnx-vm.sh`, SSH) est décrit plus bas.
> Les deux bancs sont distincts : 7.1 sert à relever la grammaire, 8.0 sert à **prouver**.

## 1. Ce que le banc produit

`python3 banc/preuve-emulateur.py` écrit `preuve/emulateur/evidences.json` : pour chaque
étape de `banc/commandes-emulateur.json`, la sortie **dans l'invité**, la sortie **sur
l'hôte**, le verdict `identique_octet_pour_octet` et les deux SHA-256.

Les étapes sont les commandes réelles du cœur C (`tamis version`, `lint`, `reach`, `explain`,
`proof`, `smt`) lancées dans l'invité sur `/proc/boot/politique.secpol` — la politique QNX 8.0
embarquée dans l'IFS — et sur le poste sur `fixtures/politique-qnx8.secpol`.

## 2. Piloter l'invité

`banc/emulateur.py` entre par le **clavier de la console VGA** (moniteur QEMU : `sendkey`) et
ressort la sortie par la **console série** : la commande est exécutée avec
`> /dev/ser1 2>&1`, donc le journal série contient exactement les octets produits — accents
intacts, contrairement au tampon VGA (qui est en charset VGA et abime les accents).

```sh
python3 banc/emulateur.py "tamis version"      # exécute et affiche la sortie
python3 banc/emulateur.py --identite            # identité de l'invité
```

La frappe est **lente** (le VGA est le canal d'entrée) : compter ~0,5 s/caractère sur une
console série redirigée. C'est pourquoi le service de preuve attend au lieu de taper dans le
vide.

## 3. Le service de preuves

`banc/service.py` expose deux routes sur `127.0.0.1` (jamais `0.0.0.0`) :

| Route | Effet |
|---|---|
| `GET /etat` | état de l'invité + liste des étapes disponibles |
| `GET /preuve/<id>` | exécute l'étape dans l'invité, la même sur l'hôte, compare |

**Sécurité** : la surface est une **liste fermée** (`banc/commandes-emulateur.json`). Le client
ne peut demander qu'un identifiant d'étape, jamais une commande libre. Le service n'écoute que
sur `127.0.0.1` ; l'exposition publique se fait par `tailscale funnel`.

```sh
systemctl --user enable --now tamis-emulateur.service   # unité : banc/tamis-emulateur.service
curl -s http://127.0.0.1:8790/etat
curl -s http://127.0.0.1:8790/preuve/reach
```

Exposition publique (le site est statique, il ne peut pas joindre `127.0.0.1`) :

```sh
tailscale funnel --bg --https=8094 http://127.0.0.1:8790
# → https://r630-debian.tail3a8bdf.ts.net:8094/
```

Le CORS autorise explicitement l'origine du site (`https://tradoooo.github.io`,
`https://baptistefaisy.github.io`) — variable `TAMIS_EMUL_ORIGIN`.

## 4. Cycle de vie : l'invité ne tourne que si on le regarde

L'invité QNX 8.0 coûte ~1,5 Go de RAM et du CPU. **La vitrine** (`tamis-web.service`, port
8092) en est le seul propriétaire : elle l'arrête après 5 minutes sans visite
(`QNX_EMS_IDLE_MIN=5`) et le relance à la demande.

Conséquence : le service de preuves ne peut pas supposer que l'invité est là.
`banc/invite_cycle.py` encadre ce contrat :

1. `reveiller()` — demande un réveil à la vitrine (une visite vaut un réveil) ; systemd n'est
   qu'un filet si la vitrine ne répond pas ;
2. `attendre()` — attend la fin du scénario de démarrage (marqueur `RESUME` dans le journal
   série) puis sonde l'invité (une seule ligne tapée) avant de rendre la main ;
3. `pret()` / `etat()` — état lisible, sans toucher à la console.

> **Piège** : taper dans la console VGA *pendant* le boot mettrait les commandes en file
> d'attente dans l'invité. D'où l'attente du marqueur de fin de démarrage avant toute sonde.

## 5. Ce que le banc permet — et ce qu'il ne permet pas

**Permet** : exécuter le **vrai binaire QNX 8.0** dans une **vraie cible** et montrer que ses
verdicts sont identiques à ceux du poste, octet pour octet ; rejouer cette preuve **depuis le
site public**, à la demande d'un juge.

**Ne permet pas encore** : la boucle *prédit ↔ observé* au sens strict — QNX qui **applique**
une politique et **refuse** une opération. Il faut un `secpol.bin` compilé (`secpolcompile`,
SDP 8.0 installé sur le serveur) injecté dans l'IFS, puis `secpolmonitor` qui constate les
refus. Le harnais `oracle/run_oracle.sh` est prêt.

---

# Banc QNX 7.1 — faire tourner l'émulateur sur le serveur

Le banc QNX (invité **QNX Neutrino 7.1.0** sous QEMU) tourne sur le serveur **r630**, en
service systemd *utilisateur* (sans sudo), disponible en permanence, piloté à distance par
le pont local Tamis (`TAMIS_QNX_HOST`, voir `docs/BANC-REEL.md`).

> **L'image QNX n'est jamais commitée ni redistribuée** (`.gitignore` du dossier). Seuls le
> harnais, les empreintes et la procédure le sont — conformément à la règle du projet.

## 0. État vérifié (12/11/2026)

| | |
|---|---|
| Invité | QNX Neutrino 7.1.0, `/proc/boot/procnto-smp-instr` |
| SSH invité | OK — `uname -a` → `QNX vm1 7.1.0 2022/07/12-18:14:39EDT x86pc x86_64` |
| `secpol` | `/proc/boot/secpol.bin: No such file or directory` (rc 1) — aucune politique chargée |
| `secpolmonitor` | `Nothing to do, none of the options -a, -p, -q or -u were specified` (rc 1) |
| `libsecpol` | `/proc/boot/libsecpol.so.1` (21 139 octets) |
| Outils hôtes | `secpolcompile` / `secpolgenerate` / `secpolpush` : **absents de la cible** |
| Accélération | **TCG** (émulation logicielle) — KVM non utilisé, cf. §2 |
| Acceptation | `bash verifie.sh` → **OK 10 checks** |

## 1. Installation — **zéro sudo requis**

Le serveur n'a pas QEMU. Plutôt que d'exiger `sudo`, on extrait QEMU dans un **préfixe
utilisateur** : rien n'est installé dans le système, rien n'est modifié, et l'opération est
réversible en supprimant `~/qnx71-vm/qprefix`.

```sh
ssh r630
mkdir -p ~/qnx71-vm && cd ~/qnx71-vm
# 1. les paquets QEMU (sans docker, sans sudo)
apt-get download qemu-system-x86 qemu-system-common qemu-system-data
# 2. les bibliothèques manquantes (une par une : un nom invalide annule le lot)
for p in libaio1t64 libcapstone5 libfdt1 libnuma1 libpmem1 libslirp0 liburing2 \
         libibverbs1 librdmacm1t64 libdaxctl1 libndctl6 libnl-3-200 libnl-route-3-200 \
         seabios ipxe-qemu; do
  apt-get download "$p" >/dev/null 2>&1 && echo "  ok $p" || echo "  -- $p absent"
done
# 3. la dernière brique : libvdeplug.so.2 (son paquet n'a pas de candidat en trixie)
curl -s -O "https://deb.debian.org/debian/pool/main/v/vdeplug4/libvdeplug2_4.0.1-4_amd64.deb"
# 4. extraction
mkdir -p qprefix
for d in *.deb; do dpkg-deb -x "$d" qprefix; done
cp -a qprefix/usr/share/seabios/* qprefix/usr/share/qemu/     # ROM VGA + BIOS
```

Le harnais détecte tout seul ce préfixe (`qnx-vm.sh doctor` affiche
`qemu : …/qprefix/usr/bin/qemu-system-x86_64 (préfixe utilisateur, sans sudo)`) et positionne
`LD_LIBRARY_PATH` et `-L <prefixe>/usr/share/qemu` en conséquence.

> Pourquoi `libvdeplug2` vient de bookworm : le paquet n'existe plus en trixie, or QEMU 10 le
> déclare en dépendance directe. La bibliothèque est stable et sans dépendance lourde.

## 2. KVM — l'**optimisation** qui manque (1 commande sudo, facultative)

Sans le groupe `kvm`, QEMU bascule en **TCG** : l'invité démarre et fonctionne (c'est l'état
vérifié ci-dessus), mais le boot est plus lent et le CPU est consommé en continu.

```sh
sudo usermod -aG kvm "$USER"   # puis se reconnecter
```

(`sudo apt-get install qemu-system-x86` reste possible et donne le même résultat que le
préfixe, mais modifie le système — le préfixe est préféré.)

## 3. Image : deux exemplaires, deux empreintes

L'image vient de l'image communautaire `autosarai/qemu_qnx7_1` (Docker Hub, étiquette
`1.1.0`), extraite **sans docker** par `tools/pull-oci.py` (API registry, jeton rafraîchi sur
401, reprise par couche, extraction par `tar` système car les filtres Python refusent les
liens absolus des couches Docker).

Le disque fait 6,05 Gio mais se compresse en **21 Mio** (zstd -3, ratio 0,34 %) : le
transfert poste → serveur prend quelques secondes au lieu de ~53 minutes.

| Exemplaire | SHA-256 | Nature |
|---|---|---|
| `qnx71-fs.vierge.img` | `671a12dbfaa2aabe8e55713abe2734ab7b2b01b581e22021feeef3dafcab5e7f` | image amont intacte (11/04/2023) |
| `qnx71-fs.img` | `30c0a7bfa896ea7bcb5b3522d697c4238ca779201464ad5d6b32f44d1e3d5d5e` | exemplaire **éprouvé** (SSH opérationnel) |

⚠️ Un disque d'invité **change à chaque démarrage** (l'invité y écrit) : une empreinte
identifie un **état**, pas une version.

## 4. Démarrage persistant (service utilisateur systemd, sans sudo)

```sh
mkdir -p ~/.config/systemd/user
cp ~/qnx71-vm/tamis-qnx.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable --now tamis-qnx.service
loginctl enable-linger "$USER"        # survit à la déconnexion — vérifié : OK sans sudo
```

**Une seule façon de faire tourner l'invité** : le service systemd. Deux modes de lancement
se disputent le port de redirection SSH (`Could not set up host forwarding rule`) — c'est
pourquoi `qnx-vm.sh start` délègue désormais au service quand il existe.

## 5. Usage

```sh
./qnx-vm.sh start            # démarre via le service systemd utilisateur
./qnx-vm.sh status           # état + accélérateur
./qnx-vm.sh doctor           # diagnostic complet des prérequis
./qnx-vm.sh ssh 'uname -a'   # commande dans l'invité
./qnx-vm.sh stop             # arrête (service compris)
```

Variables de surcharge : `QNX_IMG`, `QNX_RAM`, `QNX_SMP`, `QNX_SSH_PORT`, `QNX_GUEST_IP`,
`QNX_NET`, `QNX_FORCE_ACCEL`, `QNX_MACHINE`, `QNX_TCG_THREAD`, `QNX_QEMU_PREFIX`.

### ⚠ Piège : le type de machine doit rester `pc-i440fx-8.2`

Cette image QNX 7.1 monte ses partitions **QNX6** via `devb-eide` (IDE). Avec le type de
machine par défaut d'un QEMU récent, le contrôleur change et **le système de fichiers racine
ne se monte plus** : l'IFS démarre (« Startup complete ») mais *tous* les binaires échouent avec
`cannot execute - No such file or directory`, et `sshd` ne démarre jamais.

D'où le `-machine "$MACHINE"` explicite (`QNX_MACHINE`, défaut `pc-i440fx-8.2`). Diagnostic :
`python3 vga-dump.py` lit le tampon texte VGA de l'invité via le moniteur QEMU — c'est le seul
canal texte (l'image n'a pas de console série).

## 6. Ce que le banc 7.1 permet — et ce qu'il ne permet pas

**Permet** : vérifier sur un vrai QNX les noms d'abilities, le comportement réel de `secpol`
et `secpolmonitor`, la présence de `libsecpol`, l'absence des outils hôtes, le chemin
`/proc/boot/secpol.bin`. Résultats publiés dans `preuve/qnx71/`.

**Ne permet pas encore** : la boucle *prédit ↔ observé* (Tamis annonce un refus, QNX le
confirme en s'exécutant). Il faut `secpolcompile` pour produire un `secpol.bin` et l'insérer
dans l'IFS — or `secpolcompile` est un **outil hôte du SDP**, absent de l'invité **et** du
conteneur amont (vérifié : aucun `qcc`, aucun outil `secpol*` hôte, aucune doc HTML).
Il manque donc la **licence QNX SDP 8.0** (gratuite, action humaine). Le harnais
`oracle/run_oracle.sh` est prêt et sort `3` (prérequis manquant) en attendant.
