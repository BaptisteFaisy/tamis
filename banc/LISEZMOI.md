<!-- SPDX-License-Identifier: MIT -->
# Banc QNX — faire tourner l'émulateur sur le serveur

Le banc QNX (invité **QNX Neutrino 7.1.0** sous QEMU) est déporté sur le serveur **r630** :
il y reste disponible en permanence, sans monopoliser la RAM du poste de dev, et le pont
local Tamis le pilote à distance (`TAMIS_QNX_HOST`, voir `docs/BANC-REEL.md`).

> **L'image QNX n'est jamais commitée ni redistribuée** (`.gitignore` du dossier). Seuls le
> harnais, les empreintes et la procédure le sont — conformément à la règle du projet.

## 1. Deux commandes `sudo` (les seules, à faire une fois)

Le serveur n'a ni QEMU ni l'accès à KVM pour l'utilisateur. Il faut :

```sh
sudo apt-get update && sudo apt-get install -y qemu-system-x86
sudo usermod -aG kvm "$USER"      # puis se déconnecter/reconnecter (ou newgrp kvm)
```

**Sans la seconde commande**, QEMU bascule en émulation logicielle **TCG** : ça fonctionne
(le harnais le détecte et le signale) mais le démarrage prend plusieurs minutes au lieu de
~30 s, et ça consomme du CPU en continu.

Vérification : `./qnx-vm.sh doctor` doit afficher `kvm : accessible`.

## 2. Installation sur le serveur

```sh
ssh r630
mkdir -p ~/qnx71-vm && cd ~/qnx71-vm
# déposer ici qnx-vm.sh, qnx-ssh (depuis banc/ du dépôt) et l'image qnx71-fs.img
chmod +x qnx-vm.sh qnx-ssh
./qnx-vm.sh doctor
```

L'image vient de l'image communautaire `autosarai/qemu_qnx7_1` (Docker Hub, étiquette
`1.1.0`), extraite **sans docker** par `tools/pull-oci.py` (API registry + `curl`/`tar`).
Le disque en ressort compressé à **21 Mio** (zstd -3) pour 6,05 Gio : le transfert
poste → serveur prend quelques secondes au lieu de ~53 minutes.

| Exemplaire | SHA-256 | Nature |
|---|---|---|
| `qnx71-fs.vierge.img` | `671a12dbfaa2aabe8e55713abe2734ab7b2b01b581e22021feeef3dafcab5e7f` | image amont intacte (11/04/2023) |
| `qnx71-fs.img` | `30c0a7bfa896ea7bcb5b3522d697c4238ca779201464ad5d6b32f44d1e3d5d5e` | exemplaire **éprouvé** (SSH invité opérationnel sur `192.168.100.10`) |

⚠️ L'empreinte d'un disque d'invité **change à chaque démarrage** (l'invité écrit dessus) :
une empreinte identifie un **état**, pas une version. C'est pour ça que les deux exemplaires
sont conservés et nommés.

## 3. Démarrage persistant (aucun sudo : service *utilisateur*)

```sh
mkdir -p ~/.config/systemd/user
cp banc/tamis-qnx.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable --now tamis-qnx.service
loginctl enable-linger "$USER"        # survit à la déconnexion — vérifié : fonctionne sans sudo
systemctl --user status tamis-qnx.service
```

## 4. Usage

```sh
./qnx-vm.sh start            # arrière-plan, KVM ou TCG automatique
./qnx-vm.sh status           # état + accélérateur choisi
./qnx-vm.sh doctor           # diagnostic complet des prérequis
./qnx-vm.sh ssh 'uname -a'   # commande dans l'invité
./qnx-vm.sh stop
```

Réseau : mode utilisateur QEMU (aucun `sudo`, aucun `tap`), avec redirection
`127.0.0.1:2225 → 192.168.100.10:22`. Variables de surcharge : `QNX_IMG`, `QNX_RAM`,
`QNX_SMP`, `QNX_SSH_PORT`, `QNX_GUEST_IP`, `QNX_NET`, `QNX_FORCE_ACCEL`.

## 5. Ce que le banc permet — et ce qu'il ne permet pas

**Permet** : vérifier sur un vrai QNX les noms d'abilities, le comportement réel de `secpol`
et `secpolmonitor`, la présence de `libsecpol`, l'absence des outils hôtes, le chemin
`/proc/boot/secpol.bin`. Résultats publiés dans `preuve/qnx71/`.

**Ne permet pas encore** : la boucle *prédit ↔ observé* (Tamis annonce un refus, QNX le
confirme en s'exécutant). Il faut `secpolcompile` pour produire un `secpol.bin` et l'insérer
dans l'IFS — or `secpolcompile` est un **outil hôte du SDP**, absent de l'invité **et** du
conteneur amont (vérifié : aucun `qcc`, aucun outil `secpol*` hôte, aucune doc HTML).
Il manque donc la **licence QNX SDP 8.0** (gratuite, action humaine). Le harnais
`oracle/run_oracle.sh` est prêt et sort `3` (prérequis manquant) en attendant.
