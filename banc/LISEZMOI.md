<!-- SPDX-License-Identifier: MIT -->
# Banc QNX — faire tourner l'émulateur sur le serveur

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

## 6. Ce que le banc permet — et ce qu'il ne permet pas

**Permet** : vérifier sur un vrai QNX les noms d'abilities, le comportement réel de `secpol`
et `secpolmonitor`, la présence de `libsecpol`, l'absence des outils hôtes, le chemin
`/proc/boot/secpol.bin`. Résultats publiés dans `preuve/qnx71/`.

**Ne permet pas encore** : la boucle *prédit ↔ observé* (Tamis annonce un refus, QNX le
confirme en s'exécutant). Il faut `secpolcompile` pour produire un `secpol.bin` et l'insérer
dans l'IFS — or `secpolcompile` est un **outil hôte du SDP**, absent de l'invité **et** du
conteneur amont (vérifié : aucun `qcc`, aucun outil `secpol*` hôte, aucune doc HTML).
Il manque donc la **licence QNX SDP 8.0** (gratuite, action humaine). Le harnais
`oracle/run_oracle.sh` est prêt et sort `3` (prérequis manquant) en attendant.
