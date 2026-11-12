<!-- SPDX-License-Identifier: MIT -->
# Tamis — 100 idées pour aller plus loin

Générées le 2026-11-12. Triées par catégorie. Les **10 retenues** pour implémentation
immédiate (sans SDP, sur le socle existant) sont marquées ⭐ et listées en §11.
Critères de choix : faisable **maintenant** (la grammaire `secpol` est bloquée sur la
licence QNX), valeur pour les prix du hackathon, et construction d'un **vrai analyseur**
démontrable sans cible.

## A. Cœur analytique & preuve (1–10)
1. ⭐ Accessibilité transitive entre deux types avec contre-exemple rejouable pas à pas (TAM-04)
2. ⭐ Certificat de non-interférence : absence de chemin **prouvée** (parcours exhaustif, compteur de visites publié) (TAM-05/20)
3. ⭐ Minimisation prouvée équivalente sur un corpus de 10 000 décisions (TAM-07)
4. ⭐ Diff sémantique : élargissements/réductions de privilège entre deux politiques (TAM-08)
5. Encodeur SMT-LIB2 → Z3 : prouver équivalence/minimalité/inaccessibilité par solveur
6. Raffinement : prouver que la politique raffine celle du BSP fournisseur
7. Détection de fuites de privilège aux frontières de composition
8. Graphe `settypeid`/`gain_priv` : cycles, élévations atteignables (trou S8)
9. Validation à froid des plages nommées : existence, disjonction, couverture (TAM-13/S13)
10. Treillis de privilèges : règles dominées par d'autres

## B. Surface QNX-spécifique (11–20)
11. Modéliser les ~70 abilities statiques + les 2 jeux root/non-root
12. Abilities dynamiques custom (préfixes `fsevmgr/`, `network/`, `vfs/`)
13. Pathtrust : vérifier l'exécution de code fiable (`path_trust`, `procnto -qt`)
14. Fonctions fortifiées : couverture des checks de débordement
15. Portail sur `secpolgenerate` : auditer le diff incrémental avant intégration
16. Boucle `libsecpolev` : rejouer les refus runtime dans le modèle statique (prédit/réel)
17. Croisement `/dev/secpolgenerate/unused` + `/errors` → candidats suppression justifiés (TAM-15)
18. `security_matrix` : mapper les propriétés prouvées sur la matrice QNX
19. `mkqnximage` mode démo sécurité : politique de référence préconstruite
20. `secpollaunch`/`secpol-preload` : modéliser les transitions de type au spawn

## C. Visualisation & UX (21–30)
21. ⭐ Visualiseur HTML du graphe de privilèges, filtrable, contre-exemple en surbrillance (TAM-23)
22. Terrain de jeu WASM : coller une politique, rapport en 5 s, zéro installation
23. ⭐ CLI ergonomique : `tamis lint|reach|minimize|diff|view`
24. Serveur de langage `.secpol` (LSP) : complétion, erreurs en direct
25. Sortie HTML interactive du rapport d'audit
26. ⭐ Mode « explique-moi ce refus » : lignes sources, chaîne de règles coupables, correctif minimal
27. Diff visuel côte-à-côte de deux politiques
28. Vue « budget de privilège » par type (heat map)
29. Graphique de l'évolution des privilèges entre versions
30. Accessibilité/thème du visualiseur

## D. CI/DevOps (31–40)
31. `tamis-ci` : commente les PR avec le diff sémantique (TAM-17)
32. Bloque une PR sur tout élargissement de privilège non approuvé
33. GitHub Action marketplace (`action.yml` composite)
34. Pre-commit hook : `tamis lint` avant chaque commit
35. Bot de revue exigeant une approbation nommée sur élargissement
36. Intégration GitLab CI / Jenkins
37. Rapport JUnit XML pour CI génériques
38. Gate de non-régression vs la politique de la branche cible
39. Badge « policy: audited » pour le README
40. Webhook de recertification à chaque push

## E. Tests & robustesse (41–50)
41. ⭐ Fuzz du moteur sous ASan/UBSan, entrées déterministes par graine (TAM-11)
42. ⭐ Mutants : ≥10 mutations des moteurs, chacune tuée (validation niv. 6)
43. Property-based testing du modèle (graphes valides aléatoires)
44. Différentiel C↔Python étendu à tous les moteurs (pas seulement le rendu)
45. Banque de 40 politiques fautives avec erreur attendue (TAM-02)
46. 12 chemins de privilège dirigés + 8 paires avant/après
47. 6 jeux de plages nommées cassés
48. Test de non-régression sur corpus de 400+ règles
49. Couverture de code (gcov/lcov) avec seuil exigé
50. Test de performance : 10 000 règles sous seuil

## F. Certification & audit (51–60)
51. ⭐ Rapport d'exhaustivité : périmètre de complétude + **angles morts bornés** (Palier 1)
52. Mapping ISO 26262 des propriétés prouvées
53. Mapping IEC 62304 (médical)
54. Mapping EN 50128 (ferroviaire)
55. Mapping UNECE R155 (cybersécurité automobile)
56. Artefacts d'audit signés (empreinte + horodatage)
57. Traçabilité exigence→propriété→preuve (matrice)
58. Gabarit de dossier de sécurité pré-rempli
59. Angles morts déclarés (trou S4) signalés non couverts (TAM-21)
60. Export « preuve » régénérable avec somme de contrôle

## G. Menaces & sécurité (61–70)
61. Catalogue de menaces STRIDE exécutable → requêtes formelles sur le graphe
62. Arbres d'attaque traduits en requêtes + rapport de couverture
63. Scénarios d'attaque rejoués pas à pas
64. Détection + animation de chemins d'attaque réels
65. Correctif minimal proposé pour tuer un chemin (moindre privilège)
66. MITRE ATT&CK for ICS/automotive : mapping des capacités
67. ⭐ Lint moindre privilège : processus non typés, `default_spawn_type` manquant, abilities sur-dimensionnées (TAM-06)
68. Points d'attachage trop larges (`allow_attach` wildcards)
69. Règles en contradiction avec un concept de sûreté déclaré
70. Score de surface d'attaque par type

## H. Écosystème & communauté (71–80)
71. Corpus public de politiques (exemples SDP + projets ouverts)
72. ⭐ Codelab 4 chapitres « auditer une politique QNX » (Best Codelab bonus)
73. Codelab « de vulnérable à durci en 20 minutes »
74. Contribution amont au dépôt QNX (TAM-24)
75. Docs EN/FR complètes
76. Packaging : `make install`, binaire statique
77. Publication du jeu de données + résultats reproductibles
78. Site de démonstration (GitHub Pages) avec le visualiseur
79. Vidéo 3 min (attaque rejouée, correctif, le chemin meurt)
80. Exemples par domaine (automotive, médical, industriel)

## I. Performance & échelle (81–90)
81. Analyse incrémentale : ne recalculer que le sous-graphe modifié
82. Cache des verdicts par empreinte de politique
83. Parallélisation des requêtes d'accessibilité
84. Index de types pour lookup O(1)
85. Compression du graphe (BDD) pour grandes politiques
86. Streaming : analyser sans tout charger
87. Mémoïsation des chemins
88. Profilage + benchmark de référence
89. Politiques de 100 000 règles
90. Mémoire bornée (aucune allocation non maîtrisée)

## J. Avancé / horizon 3 mois (91–100)
91. Noyau de preuve Lean 4 : formaliser la sémantique d'accessibilité
92. Flux indirects IPC : `MsgSend`/`ConnectAttach`, passation de descripteurs
93. Multi-images & hyperviseur : politiques par partition, interférences inter-invités
94. QFS et conteneurs typés
95. Provenance & recertification : raison/ticket/propriétaire/expiration par règle
96. Répétition de déploiement : simuler `secpolpush` avec plan de retour arrière
97. Non-interférence informationnelle (flux explicites + implicites)
98. Fusion à trois voies assistée (TAM-18)
99. Stabilisateur : aligner N politiques générées par clé sémantique (TAM-19/S6)
100. Extension aux objets non-canaux : mémoire partagée, connexions (TAM-22/S15)

## 11. Les 10 retenues (implémentées cette session)

Critère : démontrer un **vrai analyseur** de bout en bout **sans le SDP** (le modèle est
construit depuis des fixtures ; le parseur `secpol` viendra se brancher dessus à la levée du
blocage licence). C'est l'argument décisif du projet : « le seul démontrable sans cible ».

| # | Idée | Exigence | Pourquoi |
|---|---|---|---|
| 1 | Accessibilité transitive + contre-exemple | TAM-04 | La fonction phare de la vidéo |
| 2 | Certificat de non-interférence (preuve) | TAM-05/20 | « prouvé, pas seulement non trouvé » — le différenciateur |
| 3 | Minimisation prouvée équivalente | TAM-07 | Démo 400→212 règles, 10 000 décisions |
| 4 | Diff sémantique | TAM-08 | Non-régression de privilège en CI |
| 5 | Lint moindre privilège | TAM-06 | « 88 règles mortes, 2 processus non typés » |
| 6 | CLI `tamis` | — | Rend l'outil utilisable, produit cohérent |
| 7 | Visualiseur HTML du graphe | TAM-23 | Vote Community Choice |
| 8 | Rapport d'exhaustivité + « explique-moi » | TAM-21 | Distingue un outil honnête d'un linter |
| 9 | Mutants + fuzz | TAM-11 | Prouve la qualité des tests |
| 10 | Codelab chapitre 1 | Best Codelab | Bonus 3 500 $ |

Infrastructure implicite : **couche modèle** (`core/model.h` + `model.c`) construite depuis un
format de fixture provisoire, sur laquelle tous les moteurs tournent — le parseur `secpol`
(lot L1) produira le même modèle dès la grammaire gelée.
