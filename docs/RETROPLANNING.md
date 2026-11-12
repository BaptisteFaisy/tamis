<!-- SPDX-License-Identifier: MIT -->
# Rétroplanning — du 12/11 au 15/12/2026

**Rédaction : 12 novembre 2026. 33 jours restants** jusqu'au dépôt du candidat au
QNX Global Hackathon 2026 (dépôt public <https://github.com/BaptisteFaisy/tamis>,
site <https://baptistefaisy.github.io/tamis/>).

**Règle qui gouverne ce plan :** aucune nouvelle fonctionnalité après le gel du 05/12 ;
à partir du gel, le temps ne sert qu'à la documentation, la relecture, la vidéo et la
soumission.

Ce document est la seule référence de dates du projet. Chaque jalon a un **livrable
vérifiable** — un fichier, une commande, un nombre — jamais une intention. Si un livrable
n'est pas vérifiable, le jalon n'est pas un jalon.

---

## 1. Jalons

| Date | Jalon | Livrable vérifiable | Statut |
|---|---|---|---|
| 12/11 | État de départ (socle, preuves, banc, catalogue) | `make test` → **9 969** vérifications vertes ; `docs/1000-idees-consolidation.md` = 1 000 idées, `docs/top10-consolidation.md` = top 10 ; `core/src/smt.c` validé par **Z3 4.13.3** (`unsat`/`sat` corrects) ; banc QNX 7.1 sous QEMU en service sur r630 + `banc/sdp.sh` + `tools/extract_grammar.py` | fait |
| 13/11 | Décision sur la licence SDP 8.0 | `docs/DECISIONS.md` : une entrée **datée du 13/11** « Licence SDP 8.0 : demandée / en attente / renoncée », avec la **date-butoir du 20/11** inscrite dans l'entrée | humain — en attente |
| 14/11 | Gel du contrat de grammaire **si** licence obtenue, sinon plan B acté | `grep -c 'À VÉRIFIER' docs/GRAMMAR.md` → **0** (licence) ; sinon entrée datée « plan B : mode sous-ensemble strict, sources attestées QNX 7.1 » dans `docs/DECISIONS.md` + §7 de `docs/GRAMMAR.md` complet | conditionnel — dépend du 13/11 |
| 17/11 | Revalidation de la grammaire 7.1 → 8.0 | `banc/revalide.sh` exécuté → rapport `preuve/qnx71/revalidation.txt` à **0 écart** ; `docs/GRAMMAR.md` : une production par page source | dépend du SDP |
| 21/11 | **Date-butoir licence** : bascule officielle en plan B | soit `docs/GRAMMAR-auto.md` extrait du SDP 8.0 (fichier non vide, une entrée par production), soit entrée « plan B » datée dans `docs/DECISIONS.md` et §« Si la licence n'arrive pas » de `docs/SDP.md` coché | décision — humain |
| 25/11 | Différentiel `secpolcompile` (TAM-12) **ou** statut assumé | `bash oracle/run_oracle.sh` → code **0** (accord total sur le corpus) ; à défaut code **3** documenté au niveau 3 de `docs/VALIDATION.md` et dans l'onglet *Grammar & contract* du site | conditionnel |
| 28/11 | Dossier de soumission complet + traçabilité des affirmations | `docs/devpost-form.md` : **13 champs**, 0 « TBD » ; `docs/TRACEABILITE.md` : chaque affirmation QNX du dossier pointe une source (page de doc ou artefact `preuve/qnx71/`) | à faire |
| 02/12 | Gel du périmètre | `docs/PERIMETRE.md` : liste exacte des fonctions livrées (13 verdicts, 3 rendus, 6 moteurs, 8 commandes CLI) ; `site/content.json` : 21 fonctionnalités, chacune avec son état | à faire |
| 05/12 | **GEL FONCTIONNEL** | `git diff --name-only <avant-gel>..HEAD` : **0** fichier de `core/`, `host/`, `tools/` modifié autrement que par un correctif couvert par un test ; `make test` ≥ 9 969 | jalon |
| 10/12 | Checklist « prêt à soumettre » (20 points binaires) | `docs/CHECKLIST-SOUMISSION.md` : **20 lignes**, chacune avec la commande qui la vérifie | à faire |
| 12/12 | **Vidéo montée + soumission Devpost** | `media/tamis-3min.mp4` ≤ **2:58** + sous-titres SRT FR/EN + SHA-256 dans `media/SHA256SUMS` ; soumission Devpost déposée (URL dans `docs/CHECKLIST-SOUMISSION.md`) | humain — critique |
| 13/12 | Relecture croisée | `docs/RELECTURE.md` : 2 relectures (README + dossier Devpost + site + vidéo), **0 constat bloquant ouvert** | à faire |
| 14/12 | Gel de la soumission (J‑1) | `docs/RETROPLANNING.md` §2 mis à jour : liste finale des points ouverts, chacun avec un propriétaire et une date | à faire |
| 15/12 | **Dépôt** | tag `v1.0.0` présent sur `main` (`git tag -l v1.0.0`) ; README pointant sur la version soumise ; commit de soumission | jalon |

---

## 2. Écart

| Jalon prévu | Avancement réel | Écart | Action corrective |
|---|---|---|---|
| Encodeur SMT-LIB2 (n°1 du top 10) | fait et **validé par Z3 4.13.3** (`core/src/smt.c`, `core/tests/test_smt.c`) | **en avance** | aucune ; figer la preuve dans le dossier Devpost (rejouable : `z3 -smt2 fichier.smt2`) |
| Site bilingue FR/EN, 7 onglets | fait et publié (workflow `pages.yml`, source unique `site/content.json`) | **en avance** | ne plus y toucher après le 05/12 ; toute retouche passe par `make site` |
| Banc QNX réel (QEMU 7.1) + pont 127.0.0.1 | en service sur r630, 15 vérifications de pont (`tests/test_bridge.sh`) | **en avance** | enregistrer les captures **avant le 10/12** : le temps de tournage est le seul qui ne se rattrape pas |
| Catalogue 1 000 idées + top 10 | fait le 12/11 (`docs/1000-idees-consolidation.md`, `docs/top10-consolidation.md`) | **en avance** | alimenter le backlog, jamais le code |
| Licence QNX SDP 8.0 | **non demandée** au 12/11 (blocage humain, ~10 min + téléchargement) | **en retard — humain** | compte myQNX avant le **20/11** ; sinon plan B acté le 21/11 (§1) |
| Parseur du vrai langage `secpol` | non écrit — mode sous-ensemble strict assumé (`docs/DECISIONS.md` D-004) | dépendant de la licence | gel 14/11 si licence ; sinon parseur écrit **uniquement** sur le sous-ensemble attesté sur 7.1 |
| Différentiel `secpolcompile` (TAM-12) | harnais écrit, `oracle/run_oracle.sh` sort **3** (prérequis manquant) | dépendant du SDP | lancer dès l'obtention ; sinon documenter le niveau 3 comme optionnel partout (site, dossier, `docs/VALIDATION.md`) |
| Vidéo 3 min | script minuté prêt (site + §10 du dossier Devpost), **aucune prise de vue** | **en retard — humain** | tournage avant le **10/12** ; repli 90 s (n°3 du top 10) monté en 2 h |
| Soumission Devpost | dossier 13 champs prêt (`docs/devpost-form.md`), **compte d'équipe à créer** | dépendant du compte | créer le compte d'équipe **avant le 05/12** (il faut 3 jours de marge d'invitation/validation) |
| Chiffres publiés | 3 valeurs coexistent : README « 810 », `site/content.json` « 9 876 », décompte réel **9 969** | incohérence | une seule source de vérité : `make site` régénère `site/` et `docs/devpost-form.md` ; aligner avant le 05/12, ne plus jamais recopier un nombre à la main |
| Seuil de vérifications | 9 969 vertes ; cible de soumission 10 000 | **−31 vérifications** | +31 cas limites (nouveaux tests, aucune fonctionnalité) d'ici le 05/12, puis non-régression jusqu'au 15/12 |

---

## 3. Dépendances humaines (3 actions, porteur du projet)

**1. Licence QNX SDP 8.0 — avant le 20/11.**
Compte myQNX + téléchargement + installation (≈ 10 min de main, le reste est du
téléchargement). *Conséquence précise du retard :* au 21/11 le plan B est acté par écrit —
`docs/GRAMMAR.md` reste marqué « À VÉRIFIER » sur les points non attestés, le parseur est
écrit sur le seul sous-ensemble attesté sur QNX 7.1, et le différentiel TAM-12 reste non
exécuté (niveau 3 de `docs/VALIDATION.md`, `oracle/run_oracle.sh` sort 3). Le projet reste
présentable, mais la boucle *prédit ↔ observé* (Tamis annonce un refus, QNX le confirme)
n'existe pas, et le dossier Devpost doit assumer « grammar TO VERIFY ». Chaque jour de
retard au-delà du 14/11 repousse le jalon 17/11 d'autant.

**2. Tournage de la vidéo — avant le 10/12.**
Plans : (a) CLI en terminal sur une politique, (b) playground du navigateur,
(c) banc QNX réel via le pont, (d) preuve solveur. *Conséquence précise du retard :* la
version 90 s (n°3 du top 10) est montée en 2 h mais ne remplace pas l'exigence « vidéo
3 minutes » ; sans prise de vue avant le 12/12, la soumission part sans la preuve vidéo —
c'est le premier motif de non-quorum identifié dans le top 10. Il faut 1 jour de tournage
+ 1 jour de montage + 1 jour de sous-titres FR/EN + 1 jour de contrôle : le 10/12 est la
dernière date utile, pas une date commerciale.

**3. Soumission Devpost — le 12/12.**
Compte d'équipe + 13 champs + lien vidéo + lien dépôt. *Conséquence précise du retard :*
Devpost n'accepte pas de soumission « après coup » pour le 15/12 ; une soumission non
déposée le 12/12 vaut **0**. Le 13/12 (relecture) et le 14/12 (gel de la soumission)
n'existent que si la soumission est déjà en ligne. Le compte d'équipe doit donc exister
avant le 05/12 : il faut 3 jours pour les invitations et validations.

---

## 4. Règles de conduite

1. **Gel du 05/12 :** plus aucune fonctionnalité. Une PR qui ajoute une commande, un
   verdict, un moteur ou un onglet de site après cette date est refusée, sans débat.
2. **Zéro nouvelle dépendance externe :** pas de paquet, pas de binaire, pas de service,
   pas de licence requis. `make test` doit rester vert sur une machine nue, sans QNX,
   sans QEMU, sans réseau.
3. **Chaque changement passe par un test :** un correctif sans test qui échoue avant et
   passe après est refusé. Le compte de vérifications ne descend jamais (9 969 aujourd'hui).
4. **Un commit par étape**, message `jalon(<date>) : <livrable>`. Pas de mise en forme
   mélangée à un correctif dans le même commit.
5. **Les 20 points de la checklist sont vérifiés la veille au soir** (le 11/12 pour le
   12/12), jamais le jour même : un point non coché la veille est une action du lendemain,
   pas une case cochée à l'arrache.
6. **Aucun chiffre publié qui ne sorte d'une commande du dépôt** (`make test`,
   `make mutants`, `make sanitize`). Les nombres recopiés à la main sont interdits.
7. **Un seul générateur :** `make site` régénère `site/` et `docs/devpost-form.md` depuis
   `site/content.json`. Ces deux cibles ne s'éditent jamais à la main.
8. **La grammaire ne s'invente pas :** tant qu'un point est « À VÉRIFIER », il le reste
   et le dit ; on ne passe jamais en « gelé » sans source datée.
9. **La licence SDP est le seul bloqueur :** 30 minutes d'attention par jour maximum tant
   qu'elle n'est pas là. Pas d'attente passive, pas de recherche de solution de contournement.
10. **Toute dépendance humaine a une date-butoir écrite et un repli écrit.** Aucune
    dépendance ne reste « en cours » : elle est datée ou elle est close.
11. **Un refus de PR de code après le 05/12 est un signal, pas un débat** : on bascule le
    temps sur la documentation, la relecture, la vidéo, la soumission.
12. **Publication atomique le 15/12 :** d'abord la soumission Devpost (déjà déposée le
    12/12), puis le tag `v1.0.0`, puis le commit de soumission. Dans cet ordre.

---

## 5. Critères de prêt (tous vrais simultanément au 12/12)

- [ ] `make test` → **≥ 9 969** vérifications vertes, 0 échec, garde de taille verte
      (`make garde` : < 500 lignes/fichier, < 50 lignes/fonction).
- [ ] `make sanitize` → 0 crash sous ASan/UBSan ; `make mutants` → **12/12** mutants tués.
- [ ] CI verte sur `main` (workflow `ci.yml`), **≤ 5 min** par job (mesuré : 24 s).
- [ ] **10 000 vérifications** vertes au moment du dépôt (9 969 au 12/11 : les 31 restantes
      sont gagnées par des tests, pas par des fonctionnalités).
- [ ] Site déployé et à jour : <https://baptistefaisy.github.io/tamis/> répond, 7 onglets,
      bilingue FR/EN, playground utilisable sans installation.
- [ ] **Vidéo présente** : fichier ≤ 2:58, sous-titres SRT FR/EN, SHA-256 publié dans le
      dépôt, lien collé dans le dossier Devpost.
- [ ] **Dossier Devpost relu** : 13 champs remplis depuis `docs/devpost-form.md`, relus
      par une seconde personne, 0 champ « TBD », lien dépôt et lien site vérifiés à la
      souris.
- [ ] **Licence obtenue** (`QNX_SDP` pointé, `banc/sdp.sh` enchaîne extraction + oracle)
      **ou** plan B **acté par écrit** dans `docs/DECISIONS.md` avec date.
- [ ] **Aucun « TODO » ouvert dans les documents publics** :
      `grep -rn 'TODO' README.md docs/ site/` → aucune occurrence ; les « À VÉRIFIER »
      restants sont assumés et listés au §7 de `docs/GRAMMAR.md`.
- [ ] Les **20 points** de `docs/CHECKLIST-SOUMISSION.md` cochés, vérifiés le 11/12 au soir.
- [ ] Banc réel revalidé le 11/12 : `bash tests/test_bridge.sh` → 15/15, VM QNX 7.1
      répondante.
- [ ] Preuve solveur rejouable par un tiers : `z3 -smt2 <fichier exporté>` redonne le
      verdict attendu.
- [ ] Dépôt public accessible et cohérent : README à jour, chiffres alignés sur la
      source unique, `docs/RETROPLANNING.md` à jour.
