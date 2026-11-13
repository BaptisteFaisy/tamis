<!-- SPDX-License-Identifier: MIT -->
# image/ — l'invité QNX 8.0 de Tamis

Tamis n'est pas seulement un outil de poste : **son cœur C compile aussi pour
QNX 8.0**, et cette image l'embarque dans une vraie cible. Le scénario est
joué **dans** l'invité, sur une politique QNX 8.0 réelle.

## Ce que contient l'image

| Élément | Rôle |
|---|---|
| `tamis` | le cœur C + la CLI, **recmpilés pour QNX 8.0** par `qcc` (`gcc_ntox86_64`) |
| `politique.secpol` | la politique QNX 8.0 de démonstration (celle que `secpolcompile` accepte) |
| `demo.sh` | le scénario : `reach`, `lint`, `explain`, `proof`, `version` |
| `pidin`, `secpolmonitor`, `slog2info` | outils QNX embarqués |

## Construire

Tout se passe sur un hôte qui a le **SDP QNX 8.0** (`~/qnx800`) et le paquet
réutilisable **`qnx-emu`** (build-image.sh, guest-qemu.sh, web.py) :

```sh
export QNX_SDP=~/qnx800
bash image/build-qnx.sh --sdp ~/qnx800 --sortie ~/tamis-img
```

Le script fait ce qui est propre à Tamis (cross-compilation, copie de la
politique et du scénario) puis appelle `qnx-emu/build-image.sh` pour la chaîne
`mkifs` → IFS. Il ne duplique rien du paquet générique.

## Démarrer

```sh
qnx-emu/guest-qemu.sh --ifs ~/tamis-img/ifs.bin \
    --serie ~/tamis-demo/serial.log --moniteur ~/tamis-demo/mon.sock \
    --qemu ~/qnx71-vm/qprefix/usr/bin/qemu-system-x86_64 \
    --prefixe ~/qnx71-vm/qprefix --detacher
```

Puis la vitrine HTTP (écran VGA, console série, injection de touches) :

```sh
QNX_EMS_SERIE=~/tamis-demo/serial.log QNX_EMS_MON=~/tamis-demo/mon.sock \
QNX_EMS_PORT=8092 python3 qnx-emu/web.py
```

Sur le serveur `r630`, les deux sont des unités systemd utilisateur
(`tamis-qemu8.service`, `tamis-web.service`) et la vitrine est exposée en HTTPS
par `tailscale funnel --bg --https=8092 http://127.0.0.1:8092`.

## Vérifier que c'est bien la cible

Dans l'invité :

```
$ tamis reach /proc/boot/politique.secpol --src app_tiers --dst pompe_critique
ERROR  TAMIS_PATH_FOUND  /proc/boot/politique.secpol  app_tiers -> pompe_critique (3 sauts) : settypeid,settypeid,settypeid
```

Les verdicts sont **les mêmes octets** que sur le poste : le cœur C est
identique, seul le binaire change de cible.

## Les pièges déjà payés

- **Le scénario n'utilise ni tube ni substitution de commande.** Sans disque,
  le `ksh` de l'invité échoue sur `pipe()` (« can't create pipe - try again ») et
  `/tmp` est un lien mort (`/var` n'existe pas). Constat fait sur la cible, pas
  supposé — et le même constat a été fait sur la démo Réplique.
- **`on -d -t /dev/con1 ksh -c /proc/boot/demo.sh`** : le shell de démarrage
  ne gère ni `$!` ni les tubes ; le scénario est donc lancé par `ksh`.
- **Le scénario joue sur les deux consoles** (VGA et série) : la VGA donne
  l'écran capturé par la vitrine, la série donne le journal texte complet.
- **`--cpu max` et le RNG virtio sont obligatoires** (cf. `qnx-emu/README.md`).
- **L'IFS se lance par `-kernel`**, jamais par le disque.
- **Les permissions** : `demo.sh` est copié en `0555` ; le `cp` doit être
  précédé d'un `rm -f` (un fichier en lecture seule ne s'écrase pas en place).
