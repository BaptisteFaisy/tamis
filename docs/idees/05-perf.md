<!-- SPDX-License-Identifier: MIT -->
# Idées de consolidation — performance et échelle (100)

1. Remplacer la recherche linéaire des types par une table de hachage ouverte, indexée par nom.
2. Ranger les règles en liste d'adjacence contiguë, pour itérer les successeurs sans balayer tout.
3. Construire un index inverse cible-règles, permettant d'énumérer les prédécesseurs rapidement.
4. Substituer au tableau de visite un jeu de bits de 64 bits, divisant la mémoire par huit.
5. Interner les noms de types et les abilities dans une table unique de chaînes partagées.
6. Remplacer les index entiers signés par des uint32, pour doubler la densité mémoire du graphe.
7. Trier les règles par couple source-cible, pour comparer deux politiques par fusion de listes.
8. Séparer les règles sans ability des règles nommées, pour accélérer la détection de redondance.
9. Précalculer le degré entrant et sortant de chaque type, stocké sur seize bits.
10. Stocker les règles en format CSR à deux passes, pour un parcours contigu des arcs.
11. Effectuer un seul parcours en largeur par source, au lieu d'un parcours par couple demandé.
12. Calculer la fermeture transitive par Warshall sur bitsets, quand le nombre de types reste modéré.
13. Représenter l'ensemble des successeurs d'un type par un bitset de 4096 bits.
14. Réduire le graphe à sa condensation, puis n'analyser que les classes de composantes.
15. Détecter les composantes fortement connexes, pour partager un seul calcul entre leurs membres.
16. Exclure du calcul toute-paire les types dépourvus de toute règle sortante.
17. Écarter les nœuds terminaux du graphe de transition, sans modifier les résultats obtenus.
18. Comparer le modèle source et le modèle minimisé par différence d'arcs, au lieu de toute-paire.
19. Vérifier l'équivalence de minimisation par différence d'atteignables, en O(E log E).
20. Borner l'analyse toute-paire par un budget de paires, et publier la fraction non couverte.
21. Invalider le cache d'atteignabilité par l'empreinte du sous-graphe modifié, non du fichier entier.
22. Déterminer le plus petit ensemble de types affectés par une règle ajoutée ou retirée.
23. Réutiliser la matrice d'atteignabilité précédente, en ne recalculant que les paires concernées.
24. Identifier chaque règle par un index stable, pour localiser une modification sans relire le texte.
25. Répertorier les règles par numéro de ligne, pour comparer deux versions sans réanalyse complète.
26. Mémoïser le résultat d'accessibilité par couple de types, invalidé par version du modèle.
27. Mémoïser le parcours par source, pour réutiliser un seul calcul sur toutes les cibles demandées.
28. Employer un cache LRU de taille fixe pour les requêtes interactives du site.
29. Consigner dans le cache la version du moteur, et le rejeter si elle diffère de l'actuelle.
30. Charger le cache au démarrage de la CLI, pour rejouer une analyse identique en moins de 50 ms.
31. Répartir les paires toute-paire entre threads, chacun détenant son propre tampon de visite.
32. Réutiliser un pool de threads existant, alloué une seule fois au démarrage du processus.
33. Paralléliser le rendu Markdown du rapport, en préservant l'ordre exact des verdicts émis.
34. Analyser plusieurs fichiers en parallèle depuis la CLI, une politique par processus enfant.
35. Protéger l'émission des verdicts par un mutex, en conservant un ordre global stable.
36. Remplacer le verrou global par des opérations atomiques sur les bitsets partagés.
37. Mesurer le gain du parallélisme sur huit cœurs, et publier la courbe temps selon le nombre de threads.
38. N'activer le parallélisme qu'au-delà d'un seuil de types, pour ne pas pénaliser les petits cas.
39. Fixer un plafond d'arène configurable, et échouer proprement dès qu'il est dépassé.
40. Compter les octets demandés par moteur, et publier le pic de mémoire dans le rapport.
41. Réutiliser un pool de tampons entre les parcours, au lieu d'allouer à chaque requête.
42. Réserver l'espace des tampons de BFS une seule fois par analyse, puis le recycler.
43. Construire le modèle à la volée, règle par règle, sans conserver le texte source en mémoire.
44. Réinitialiser l'arène entre deux analyses, avec un pointeur de reprise documenté.
45. Évaluer la taille d'arène requise avant l'analyse, et refuser de démarrer si elle dépasse.
46. Interner les libellés de règles, pour éviter de copier une chaîne par chemin construit.
47. Remplacer les pointeurs ability et id par des index sur seize bits dans la règle.
48. Ajouter une option --budget-temps, qui interrompt l'analyse et publie l'état atteint.
49. Distinguer le mode rapide, limité aux paires demandées, du mode exhaustif toute-paire.
50. Retourner un code de sortie distinct pour budget épuisé, séparé de l'absence de chemin.
51. Afficher le pourcentage de paires couvert avant épuisement du budget, dans le rapport.
52. Découper l'analyse toute-paire en lots, avec reprise possible entre deux lots consécutifs.
53. Sauvegarder l'état d'analyse après chaque lot, pour reprendre après une interruption.
54. Fixer un seuil par défaut de trente secondes, réglable par variable d'environnement.
55. Encoder l'ensemble des types atteignables par un diagramme de décisions binaire.
56. Repérer les sous-graphes isomorphes, pour partager leur représentation compacte.
57. Représenter les capacités par ability sous forme de bitsets, un bit par ability.
58. Grouper les règles par ability, puis détecter les conflits par intersection de bitsets.
59. Comparer deux politiques par différence de bitsets d'abilities, plutôt que par liste de règles.
60. Vectoriser la fermeture transitive par des opérations AND, quand le nombre de types est multiple de 64.
61. Employer les instructions SSE2 pour comparer deux bitsets de 4096 bits d'un seul coup.
62. Compter les bits d'un ensemble d'atteignabilité avec popcount, sans boucle.
63. Aligner les tableaux de bitsets sur soixante-quatre octets, pour le cache et la vectorisation.
64. Dérouler la boucle interne du parcours en blocs, pour supprimer les branches par sommet.
65. Compiler le cœur avec -O3 et -flto, en vérifiant que les sorties restent identiques.
66. Ajouter une cible perf au Makefile, compilant avec -pg pour le profilage gprof.
67. Publier le profil gprof de la toute-paire, annexé au dossier de preuve.
68. Mesurer les instructions et les défauts de cache avec perf stat sur une analyse complète.
69. Publier un tableau de référence des durées, sur trois tailles de corpus documentées.
70. Échouer la CI si une analyse de référence met plus du double de son temps habituel.
71. Réduire le coût d'appel avec -fno-plt et la visibilité restreinte des symboles exportés.
72. Marquer les boucles critiques du parcours comme hot, et mesurer le gain obtenu.
73. Compiler le jumeau JavaScript avec un bundler, et garder un benchmark de référence commun.
74. Exécuter le même benchmark dans le navigateur, et publier le résultat sur le site.
75. Générer des politiques synthétiques de mille à cent mille règles, par graine reproductible.
76. Publier le générateur de corpus synthétique, pour rendre chaque benchmark rejouable par tous.
77. Mesurer la durée d'un parcours selon la densité du graphe, et tracer la courbe obtenue.
78. Tester la pire casse sur une chaîne de dix mille types, qui épuise le parcours en largeur.
79. Tester un graphe dense de mille types et cent mille arcs, pour vérifier la borne mémoire.
80. Comparer les durées du cœur C, du jumeau Python et du jumeau JavaScript sur un même corpus.
81. Analyser une politique directement depuis un flux, sans jamais la charger entière.
82. Écrire le rapport JSON en flux, sans construire la chaîne complète en mémoire.
83. Débiter le rapport par blocs de soixante-quatre kilo-octets, avec vidage progressif.
84. Accepter une politique sur l'entrée standard, pour les chaînes de traitement en CI.
85. Mesurer le débit d'analyse en mode flux, en politiques traitées par seconde.
86. Découper le jumeau JavaScript en modules, pour ne pas recharger tout le script.
87. Exécuter l'analyse dans un Web Worker, pour ne pas figer l'interface du navigateur.
88. Transférer le modèle au Worker par tableau transférable, sans copie de structure.
89. Refuser dans le terrain de jeu toute politique dépassant deux cent kilo-octets.
90. Afficher une barre de progression pendant les analyses longues, côté navigateur.
91. Documenter pourquoi l'accélération GPU a été écartée, avec le coût de transfert mesuré.
92. Comparer le coût du GPU et celui du processeur, et publier la mesure dans le dossier de preuve.
93. Renoncer au GPU pour la toute-paire, le transfert de données dépassant le gain mesuré.
94. Retenir le vectoriel SIMD sur processeur comme seule voie d'accélération retenue.
95. Justifier l'absence de GPU dans l'architecture, par un paragraphe documenté et sourcé.
96. Fournir un démon local indexant les politiques, pour une réanalyse instantanée.
97. Invalidater le cache par numéro de génération, plutôt que par comparaison de contenu.
98. Limiter la taille du cache sur disque, avec éviction du plus ancien utilisé et journal.
99. Vérifier l'égalité des verdicts entre mode exhaustif et mode rapide, sur cent paires tirées.
100. Publier le tableau de montée en échelle, avec les durées mesurées jusqu'à cent mille règles.
