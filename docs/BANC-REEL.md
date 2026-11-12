<!-- SPDX-License-Identifier: MIT -->
# Banc réel — piloter le vrai cœur C et la vraie VM QNX depuis le site

## Pourquoi un pont local

Le site <https://baptistefaisy.github.io/tamis/> est servi par **GitHub Pages : un hébergement
statique**. Il ne peut donc pas :

| Voulu | Pourquoi c'est impossible sur un site statique |
|---|---|
| Exécuter le binaire C `build/tamis` | pas de processus serveur |
| Faire tourner QEMU / une VM QNX | pas d'hyperviseur, pas de noyau |
| Servir l'image QNX (6,05 Gio) | taille prohibitive **et** licence : une image QNX n'est pas redistribuable |

Le **pont local** (`tools/tamis-bridge.py`) comble l'écart : il tourne sur **ta** machine, et le
site s'y connecte quand tu le décides. Le site public reste 100 % statique ; la puissance réelle
reste locale.

> Sans pont, **tout le Playground fonctionne quand même** : le jumeau JavaScript
> (`site/tamis-core.js`) est une implémentation indépendante, vérifiée octet pour octet contre
> le cœur C par `tests/test_js.sh` (13 scénarios, 0 divergence).

## Démarrage

```sh
cd tamis
make cli                        # construit le vrai binaire C (sinon le pont le fait au 1er appel)
python3 tools/tamis-bridge.py   # imprime l'URL et le JETON
```

Puis, sur le site : onglet **Banc réel** → coller le jeton → **Connecter**.

Options : `--port 8799` (changer le port), `--vm DOSSIER` (banc QNX ailleurs que
`~/Bureau/qnx71-vm`), `--token-fixe JETON` (tests uniquement).

## Modèle de sécurité (quatre couches cumulatives)

Un serveur local qui exécute des commandes est dangereux s'il est mal tenu. Ici :

1. **Écoute sur `127.0.0.1` uniquement** — jamais sur `0.0.0.0`. Injoignable depuis le réseau
   local. Vérifié par `tests/test_bridge.sh` (il tente une connexion sur l'interface réseau et
   exige qu'elle échoue).
2. **Jeton aléatoire** (`secrets.token_urlsafe(18)`) imprimé au démarrage et exigé dans
   l'en-tête `X-Tamis-Token` sur tout sauf `/ping`. **Sans jeton, un site tiers malveillant que
   tu visites ne peut rien faire, même en devinant le port.**
3. **En-tête `Origin` contrôlé** : seuls `https://baptistefaisy.github.io`, `http://localhost:*`
   et `http://127.0.0.1:*` sont admis (403 sinon).
4. **Aucune commande libre** : `cmd` et `format` sont sur liste blanche, `src`/`dst`/`label`
   sont validés par expression rationnelle, les politiques passent par un **fichier temporaire**
   (jamais en argument), `subprocess` est appelé **sans shell** (liste d'arguments), le corps de
   requête est borné à 1 Mio. Côté QNX, le navigateur choisit un **nom de préréglage**, jamais
   une commande.

En-têtes CORS + `Access-Control-Allow-Private-Network: true` pour la politique *Private Network
Access* de Chromium (une page publique qui appelle localhost).

## Ce que le banc permet

### 1. Différentiel en direct : vrai C ↔ jumeau navigateur

Même politique, même commande, deux implémentations. Le site affiche les deux sorties côte à
côte et dit si elles sont **identiques**. C'est la preuve TAM-09, rejouable sous tes yeux — si
quelqu'un modifie le cœur C ou le jumeau JS sans mettre l'autre à jour, l'écran passe au rouge.

La comparaison se fait en **JSON**, le format autoritaire (une ligne par verdict, ordre des clés
figé, échappement précisé au caractère près dans `CONTRACT.md` §5.2).

Le nom de fichier rapporté dans les verdicts est forcé à `policy.mpf` via `--label` : le chemin
temporaire réel ne fuit pas, les deux sorties sont donc comparables.

### 2. Banc QNX réel

| Préréglage | Ce qu'il montre |
|---|---|
| `uname` | la version réelle du noyau QNX |
| `boot` | `/proc/boot` : `libsecpol.so.1`, `ability -> on`, présence/absence de `secpol.bin` |
| `secpol` | l'outil officiel, et son comportement réel sans politique chargée |
| `secpolmonitor` | le moniteur officiel et ses options réelles |
| `libsecpol` | la bibliothèque de politique |
| `hosttools` | la preuve que `secpolcompile`/`secpolgenerate`/`secpolpush` sont **hôtes** |
| `procnto` | le noyau en cours |

Démarrage ≈ 1 min (le pont attend que SSH réponde, borné à 180 s), arrêt immédiat.

## Ce que le banc ne prouve pas encore

La boucle **prédit ↔ observé** — Tamis annonce un refus, QNX le confirme en s'exécutant — exige
`secpolcompile` pour produire un `secpol.bin`, puis son insertion dans l'IFS. `secpolcompile`
est un **outil hôte du SDP**, et le SDP 8.0 n'est pas installé (licence gratuite, action
humaine). Le harnais `oracle/run_oracle.sh` est déjà écrit et sort `3` (prérequis manquant) en
attendant. C'est **le seul maillon manquant** de la démonstration complète.

## Dépannage

| Symptôme | Cause probable | Solution |
|---|---|---|
| « pont injoignable » | le pont n'est pas lancé, ou autre port | lancer `tools/tamis-bridge.py`, vérifier le port |
| « jeton refusé » (401) | jeton recopié avec une espace, ou pont redémarré | recoller le jeton fraîchement imprimé |
| 403 Origine refusée | le site est servi depuis un autre hôte | servir le site depuis `localhost`, ou ajouter l'origine |
| VM « non configurée » | dossier introuvable | `--vm /chemin/vers/qnx71-vm` ou `TAMIS_QNX_VM=…` |
| `qnx/boot` dépasse le délai | l'invité met du temps ou SSH n'est pas up | réessayer ; `./qnx-vm.sh status` côté dépôt du banc |

## Tests

```sh
bash tests/test_bridge.sh    # 15 vérifications : comportement + les 4 couches de sécurité
make test                    # inclut test_bridge.sh et le smoke test du site (63 vérifications)
```
