<!-- SPDX-License-Identifier: MIT -->
# Les 10 meilleures idées de consolidation (sur 1000)

Catalogue complet : [`docs/1000-idees-consolidation.md`](1000-idees-consolidation.md)
(1 000 idées, 10 thèmes, sans doublon). **Sélection ci-dessous** : ce que je lancerais
demain, dans cet ordre, avec la raison et le jalon.

Critères : impact perçu par les juges le 15/12 · faisabilité **sans** la licence SDP ·
effort vs valeur · risque de non-quorum.

| # | Idée | Thème | Pourquoi celle-là | Effort |
|---|---|---|---|---|
| **1** | **Encoder le graphe d'accessibilité en relation SMT-LIB2 et prouver l'absence de chemin par solveur** (export rejouable par un tiers) | preuve | C'est le critère « implémentation non triviale » du règlement, et le seul moyen de transformer « j'ai cherché » en « un solveur l'a vérifié ». Fonctionne sans Z3 installé (on émet du SMT) ; avec Z3, la preuve devient exécutable. | 1 j |
| **2** | **Tourner la vidéo de 3 minutes** (plans filmés séparément, sous-titres SRT FR/EN, durée ≤ 2:58, empreinte SHA-256 dans le dépôt) | démo | C'est un livrable **exigé** et le premier risque de non-quorum : rien ne vaut un dossier parfait sans la vidéo. Les sous-titres EN字幕 protègent les juges non francophones. | 1 j (tournage) |
| **3** | **Version de repli de 90 secondes**, centrée sur la seule preuve de non-interférence | démo | Un jury pressé doit comprendre en 90 s. Une seule idée, une seule preuve, zéro jargon. C'est l'assurance du livrable n°2. | 2 h |
| **4** | **Capturer la session du banc QNX réel et la démonstration du pont (C réel == jumeau JS)** | démo | Répond à la question qui élimine les concurrents : « est-ce que c'est réel ? ». La sortie QNX 7.1.0 et le verdict identique affiché côte à côte sont des preuves qu'un concurrent ne peut pas produire. | 3 h |
| **5** | **Rétroplanning daté du 12/11 au 15/12**, jalon vérifiable par semaine + tableau d'écart | pilotage | 33 jours, un seul livrable qui compte. Le tableau d'écart est ce qui distingue « on avance » de « on pense avancer ». | 1 h |
| **6** | **Checklist « prêt à soumettre » de 20 points binaires**, testée la veille | pilotage | Transforme la soumission en une liste objective plutôt qu'un ressenti. Laisse zéro place à l'oubli de dernière minute (clé d'API, lien mort, vidéo trop longue). | 1 h |
| **7** | **Fixer la date-butoir du SDP au 20/11, sinon basculer officiellement en plan B** | pilotage | Débloque la décision : attendre la licence sans deciding, c'est risquer de découvrir au 10/12 que le plan B n'est pas écrit. Le plan B existe déjà (grammaire attestée sur QNX 7.1, oracle déclaré optionnel). | 30 min |
| **8** | **Tableau de traçabilité de chaque affirmation QNX 8.0** (source + niveau de confiance), relu par un pair | pilotage | Le dossier Devpost est le lieu exact où une affirmation non sourcée se retourne contre nous. Le tableau rend l'honnêteté auditable et le relu par un pair attrape ce que l'auteur ne voit plus. | 2 h |
| **9** | **Script de revalidation des attestations 7.1 sur une cible 8.0** | QNX | Le SDP arrivera peut-être après le gel. Ce script fait de son arrivée un travail de 10 minutes au lieu d'une recherche. Il ferme aussi le risque « 7.1 ≠ 8.0 ». | 2 h |
| **10** | **Gel de périmètre daté** : plus aucune dépendance externe ni fonctionnalité nouvelle, et liste exacte des fonctions livrées | pilotage | Le meilleur moyen de finir est d'arrêter d'ajouter. La liste exacte des fonctions livrées devient le périmètre contractuel du dossier Devpost. | 30 min |

## Pourquoi pas les autres finalistes

- **Encodeur Lean 4 / preuves formelles** — excellent, mais hors délai et hors dépendance
  (aucun Lean installé). Le SMT-LIB2 (n°1) atteint le même critère pour un dixième de l'effort.
- **Données de test à 400+ règles** — utile pour l'échelle, mais le corpus synthétique de
  règles est un générateur, pas une exigence. Le différentiel C↔Python↔JS couvre déjà la
  correction du moteur.
- **Package Homebrew / crate crates.io / binding Go** — de la distribution, pas de la note.
  Le dépôt public + le site suffisent pour un jury.
- **Visualiseur du rapport HTML autonome** — le site bilingue et le visualiseur de graphe
  couvrent déjà la demonstration.

## Ce que je lance tout de suite

Les n°1, 5, 6, 7, 8, 10 sont faisables **immédiatement** et sans dépendance externe :
l'encodeur SMT-LIB2 (n°1) est le seul qui demande du code ; les autres sont de l'écriture
et des tableaux versionnés. Les n°2, 3 et 4 demandent la caméra et le banc — le banc QNX
tourne déjà sur le r630, il ne manque qu'un enregistrement.

Le n°9 sera written en même temps que l'extracteur de grammaire déjà en place
(`tools/extract_grammar.py`) : les deux s'activent au même moment, quand le SDP arrive.

## État d'avancement

| # | Idée | État |
|---|---|---|
| **1** | Encodeur SMT-LIB2 + preuve par solveur | **FAIT** — `core/src/smt.c`, `tamis smt`, `tests/test_smt.c`. Validé par **Z3 4.13.3** : `unsat` pour le chemin qui existe, `sat` pour l'absence de chemin |
| 2 | Vidéo 3 min | à filmer (banc QNX prêt, script minuté prêt) |
| 3 | Version 90 s | à monter |
| 4 | Capture banc QNX + pont | à enregistrer |
| 5 | Rétroplanning daté | à rédiger |
| 6 | Checklist « prêt à soumettre » | à rédiger |
| 7 | Date-butoir SDP au 20/11 | **à faire en premier** (30 min) |
| 8 | Tableau de traçabilité des affirmations | à rédiger |
| 9 | Script de revalidation 7.1 → 8.0 | à écrire (s'active avec le SDP) |
| 10 | Gel de périmètre daté | à faire en premier (30 min) |

> Note : Z3 n'est pas installé sur le poste de dev (le test bascule alors en mode
> structurel). Il a été installé **sans sudo** sur le serveur, en préfixe utilisateur —
> même méthode que QEMU (voir `banc/LISEZMOI.md`) — et la preuve a été exécutée là.
