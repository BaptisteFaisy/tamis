# Vitrine publique — preuve que la démo Tamis s'ouvre depuis internet

**Date : 2026-11-14.** Verdict : **OUI**, la vitrine de l'invité QNX 8.0 et le service des
preuves sont joignables depuis l'extérieur du serveur, en HTTPS, sans Tailscale installé
côté client.

| Surface | URL publique | Contrôle |
|---|---|---|
| Vitrine de l'invite (écran VGA + console série + touches) | <https://r630-debian.tail3a8bdf.ts.net:8443/tamis/> | HTTP 200, flux JPEG 200 |
| Service des preuves (liste fermée exécutée **dans** l'invité) | <https://r630-debian.tail3a8bdf.ts.net:8443/tamis-api/etat> | HTTP 200, JSON |
| Une preuve, par un juge | <https://r630-debian.tail3a8bdf.ts.net:8443/tamis-api/preuve/reach> | HTTP 200, `identique_octet_pour_octet: true` |

Reproductible en une commande : `banc/prouve-public.sh` (22 vérifications, rc 0).

## La fausse piste : le « hairpin NAT »

Le 13/11, le serveur ne parvenait pas à joindre son propre funnel (`:8092` répondait,
`:8094` rendait 404) et on en concluait à l'impossibilité de tester depuis la machine.
**Cause réelle, mesurée le 14/11 :**

1. **Tailscale Funnel ne publie que les ports 443, 8443 et 10000**
   (<https://tailscale.com/kb/1223/funnel>). `tailscale serve status` annonce pourtant
   « Funnel on » pour `:8092`, `:8094`, `:8095` : ces ports ne sont joignables **que depuis
   le tailnet**. Un juge sans Tailscale y voit une connexion qui expire — ce n'est pas un
   hairpin, c'est l'entrée internet qui ne les sert pas.
2. Le **404 de `:8094/`** venait du service lui-même (`banc/service.py` n'expose que
   `/etat` et `/preuve/<id>` ; la racine répond `{"erreur":"chemin inconnu"}`) : le funnel
   atteignait bien le backend. Le service n'était donc pas en panne.
3. Le test depuis la machine **marche** si on épingle la requête sur l'IP publique de
   l'entrée Funnel : le trafic sort sur internet et revient par la voie d'un juge.

## Ce qui a changé

- Montages publics sous `:8443` (port publié) : `/tamis` → vitrine (`127.0.0.1:8092`),
  `/tamis-api` → service des preuves (`127.0.0.1:8790`). L'existant `/ambre` du même port
  est intact.
- `web.py` de la vitrine : la base des URLs d'assets est **dérivée du chemin de la page**
  (`var B=`), donc une seule instance sert à la fois `:8092/` (tailnet) et `:8443/tamis/`
  (internet). Avant, les assets partaient en dur vers `/stream` et casserait sous préfixe.
- Le site pointe désormais l'iframe et l'API sur `:8443` (`site/index.html`, `site/cible.js`).
- `banc/prouve-public.sh` : le contrôle, réutilisable, avec `--nom`, `--resolver`, `--port`,
  `--etapes`, `--rapport`. Le compteur de checks est factorisé dans `banc/ck.sh` (utilisé
  par `verifie.sh` et `prouve-public.sh`).

## Preuve 1 — depuis un autre poste, hors routage tailnet (la vue du juge)

Testé depuis `baptiste-MS-7D18` (100.120.194.92), nom résolu par **1.1.1.1** et requête
épinglée sur l'IP publique (`--resolve`), donc route `via 10.97.103.107 dev enxb6aaed48f1ad`
— pas `tailscale0` :

```
entrée Funnel : 176.58.90.46
GET /tamis/          HTTP 200  4365 octets  TLS=0  text/html
GET /tamis/stream    HTTP 200  639331 octets       multipart/x-mixed-replace; boundary=frame
GET /tamis/age       HTTP 200    25 octets
GET /tamis/console   HTTP 200  5181 octets
GET /tamis-api/etat  HTTP 200   769 octets  invite actif=True pret=True étapes=version,lint,reach,explain,proof,smt
/preuve/version  HTTP 200 identique=True  invité='tamis 0.1.0'                       hote='tamis 0.1.0'
/preuve/lint     HTTP 200 identique=True  invité='WARN TAMIS_MISSING_DEFAULT_SPAWN…'  hote='WARN …'
/preuve/reach    HTTP 200 identique=True  invité='ERROR TAMIS_PATH_FOUND…'            hote='ERROR …'
/preuve/explain  HTTP 200 identique=True  invité='ERROR TAMIS_PATH_FOUND…'            hote='ERROR …'
/preuve/proof    HTTP 200 identique=True  invité='ERROR TAMIS_INTERFERENCE_POSSIBLE…' hote='ERROR …'
:8092 → timeout   :8094 → timeout        (hors liste des ports publiés : rappel du piège)
```

`TLS=0` = certificat valide (émis par Tailscale pour `<node>.ts.net`), donc le lien s'ouvre
dans un navigateur ordinaire, sans avertissement.

## Preuve 2 — depuis le serveur lui-même, par l'entrée internet

`bash banc/prouve-public.sh` sur le r630 : route `176.58.90.46 via 192.168.2.1 dev eno1`
(sortie internet, pas la boucle locale) → **22 vérifications, 0 échec**, dont les six
étapes exécutées dans l'invité et le rappel que `:8094` n'est pas public.

## Effet de bord assumé

Le premier essai de montage a basculé `:8443` en « tailnet only » pendant ~2 minutes
(les vitrines `/ambre` du même port étaient donc injoignables) : restauré immédiatement
par `tailscale funnel --bg --yes --https=8443 --set-path=/ambre http://127.0.0.1:8103`,
vérifié par `tailscale serve status` (8443 de nouveau « Funnel on », `/ambre`, `/tamis`,
`/tamis-api`) et par les tests ci-dessus. Toute modification de `serve`/`funnel` sur cette
machine doit être suivie d'un `tailscale serve status` lu en entier, pas filtré.

## Reste à décider (hors périmètre du test)

- Les montages `:8092`, `:8094`, `:8095` restent actifs « tailnet » : pratiques pour soi,
  trompeurs pour un juge. Les retirer (ou les garder, documentés) est un choix, pas un bug.
- `web.py` est partagé par cinq vitrines (ambre, glane, tamis, toile, veille) : le
  dérivé de base profite aussi à `/glane`, `/toile`, `/veille` montés sur `:8091`, mais ces
  unités doivent être redémarrées pour l'embarquer.
