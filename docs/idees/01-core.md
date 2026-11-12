<!-- SPDX-License-Identifier: MIT -->
# Idées de consolidation — cœur C et robustesse (100)

1. Rejeter toute allocation de taille nulle dans tamis_arena_alloc au lieu de renvoyer un pointeur hors tampon.
2. Aligner sur huit octets le tampon fourni par l'appelant dans tamis_arena_init avant le premier bump.
3. Exposer tamis_arena_used, tamis_arena_remaining et le sommet de consommation pour observer l'arène.
4. Ajouter tamis_arena_mark et tamis_arena_rewind pour rendre l'espace d'une analyse avortée.
5. Détecter le débordement de len+1 dans tamis_arena_strdup avant de réserver l'espace.
6. Refuser un tampon nul avec une capacité non nulle dans tamis_arena_init et documenter le contrat.
7. Vérifier le débordement de la multiplication taille par sizeof lors des allocations du modèle.
8. Compter les verdicts perdus quand le tableau d'émission est saturé au lieu de les taire.
9. Dimensionner l'arène de la CLI selon la taille réelle du fichier au lieu d'une constante fixe.
10. Séparer l'arène du modèle et celle des messages pour éviter la saturation pendant l'analyse.
11. Réserver les tableaux de chemin en un seul bloc au lieu d'une allocation par chemin.
12. Réduire l'espace d'arène du BFS en réutilisant les tableaux de visite entre deux requêtes.
13. Vérifier qu'aucun chemin d'erreur de la CLI ne laisse fuiter le tampon d'arène ni le texte.
14. Rejeter une directive type sans nom au lieu de créer un type au nom vide.
15. Accepter les commentaires indentés par des espaces, comme le jumeau JavaScript les accepte déjà.
16. Éliminer la divergence de parsing des commentaires entre C et JavaScript par un test différentiel ciblé.
17. Refuser un nom de type composé de caractères hors de l'alphabet d'identifiants documenté.
18. Refuser un identifiant de règle vide quand la directive allow ne contient qu'une arobase.
19. Refuser deux abilities sur une même ligne allow au lieu d'écraser silencieusement la première.
20. Ignorer les commentaires de fin de ligne dans les directives allow au lieu d'en faire une ability.
21. Détecter les types déclarés deux fois et émettre un verdict d'incohérence au lieu de trancher.
22. Mettre le modèle de sortie à zéro quand le parseur échoue, pour interdire tout usage partiel.
23. Propager le nom du fichier jusque dans le verdict d'erreur de syntaxe produit par le parseur.
24. Borner le parcours de la chaîne de parent dans le BFS pour éviter une boucle infinie.
25. Remplacer les indices de règle de type int par size_t pour éviter toute troncature silencieuse.
26. Initialiser toutes les sorties de tamis_reach_path avant traitement pour écarter les lectures non initialisées.
27. Signaler une politique vide ou dépourvue de types par un verdict explicite plutôt qu'un rapport vide.
28. Refuser un fichier de plus de dix mégaoctets à la CLI avec un message d'erreur explicite.
29. Émettre un verdict dédié quand une politique référence un type jamais déclaré.
30. Faire remonter l'échec d'émission d'un verdict depuis tamis_reach au lieu de l'ignorer.
31. Propager l'erreur d'émission dans tamis_lint au lieu de la confondre avec un tableau plein.
32. Séparer les retours -1 et -2 de tamis_diff pour distinguer tableau plein et arène épuisée.
33. Vérifier le retour de tamis_out_emit dans explain.c au lieu d'ignorer un verdict perdu.
34. Corriger l'explication qui lit le verdict précédent lorsque l'émission a échoué dans tamis_explain.
35. Propager l'erreur du parcours exhaustif dans la matrice toute-paire au lieu d'en déduire une absence.
36. Refuser d'annoncer une non-interférence prouvée quand le parcours a échoué et signaler l'échec.
37. Introduire une liste d'erreurs typées du cœur pour distinguer syntaxe, mémoire et type inconnu.
38. Définir un jeu de codes de retour commun à toutes les fonctions du cœur et le documenter.
39. Refuser un nom de type nul dans tamis_model_type_index au lieu de lire une chaîne invalide.
40. Vérifier les pointeurs nuls dans tamis_out_init et tamis_out_emit avant de toucher au tampon.
41. Ajouter un test qui force l'épuisement d'arène et vérifie le signalement propre de chaque moteur.
42. Faire échouer explicitement la CLI quand le nombre de verdicts dépasse la capacité allouée.
43. Distinguer dans la sortie texte une erreur d'exécution d'un verdict d'analyse de sévérité error.
44. Remplacer les retours -1 ambigus du cœur par des constantes nommées selon le type d'échec.
45. Accepter un code entier dans tamis_verdict_name pour éviter le cast hors enum indéfini.
46. Exposer tamis_verdict_count_by_code pour compter les verdicts sans parcourir le tableau.
47. Ajouter tamis_severity_rank pour trier les verdicts par gravité décroissante.
48. Ajouter un constructeur de verdict vide pour initialiser un verdict sans memset manuel.
49. Fournir un constructeur code-sévérité-message pour factoriser les sept memset dupliqués.
50. Ajouter au cœur un tri des verdicts par sévérité au lieu de le refaire dans chaque appelant.
51. Publier un en-tête tamis.h regroupant les inclusions pour simplifier l'intégration des appelants.
52. Exposer le nombre de types et de règles du modèle pour dimensionner l'affichage sans introspection.
53. Ajouter une API d'énumération des règles pour permettre à un appelant de parcourir le graphe.
54. Publier une constante de version du cœur accessible par macro pour les rapports d'audit.
55. Documenter dans chaque en-tête que les pointeurs de message sont empruntés jusqu'au rendu.
56. Écrire un exemple C minimal en tête d'en-tête montrant un appel complet du cœur.
57. Remplacer gmtime par gmtime_r dans la CLI pour éviter le tampon global non réentrant.
58. Activer -Wconversion dans les indicateurs du Makefile et corriger chaque avertissement signalé.
59. Ajouter -Wpedantic, -Wshadow et -Wcast-qual aux indicateurs de compilation du cœur.
60. Vérifier que chaque appel snprintf contrôle sa valeur de retour pour détecter les troncatures.
61. Remplacer les tampons de message fixes de 256 octets par une taille calculée sur le contenu réel.
62. Dimensionner le message d'explication sur la longueur exacte de la chaîne au lieu de 1024 octets.
63. Vérifier que le calcul de la taille de la matrice toute-paire ne peut pas déborder.
64. Vérifier que la conversion de size_t vers long dans les rendus ne peut pas dépasser la limite.
65. Introduire un type opaque de handle de type pour interdire les indices négatifs dans l'API.
66. Borner explicitement le nombre de types acceptés par le parseur et documenter cette limite.
67. Ajouter une compilation des en-têtes en C++ pour vérifier que extern C protège les symboles.
68. Tester la compilation du cœur vers WebAssembly avec clang pour valider sa portabilité.
69. Ajouter une cible embarquée sans libc pour prouver que le cœur n'a aucune dépendance implicite.
70. Refuser un format de rapport inconnu dans la CLI au lieu de retomber sur le texte.
71. Extraire la construction des messages formatés dans un module partagé par lint, diff, minimize et proof.
72. Réunifier la fonction rule_label dupliquée dans reach.c et minimize.c dans un en-tête commun.
73. Réunifier le calcul du libellé de règle entre diff.c et reach.c pour supprimer la troisième copie.
74. Déplacer l'itérateur de lignes de model.c dans un en-tête interne partagé avec le futur lexer.
75. Séparer la construction du modèle de son analyse pour qu'un vrai parseur secpol puisse le brancher.
76. Isoler le rendu JSON dans son propre module pour réutiliser l'échappement dans la vue graphe.
77. Remplacer le JSON manuel de la vue graphe par l'écrivain JSON du cœur pour échapper les noms.
78. Remplacer la recherche de type linéaire par une table de hachage construite une fois par modèle.
79. Construire un index d'adjacence par liste d'arêtes pour remplacer le balayage linéaire du BFS.
80. Remplacer la double matrice toute-paire par une fermeture transitive calculée en une seule passe.
81. Partager un index de hachage des arêtes entre diff et minimisation pour supprimer deux comparaisons quadratiques.
82. Extraire un module de statistiques d'analyse exposant nœuds visités, règles examinées et temps de parcours.
83. Corriger l'aide de la CLI qui omet la commande explain dans la liste des commandes.
84. Supprimer la constante magique de retour dupliquée par un comptage des verdicts réellement émis.
85. Exposer le compteur de visites de la preuve dans le rapport texte et dans le JSON.
86. Publier le nombre de règles examinées par la minimisation pour rendre le verdict auditable.
87. Afficher dans le rapport Markdown la sévérité effective de chaque verdict et non celle par défaut.
88. Ajouter l'espace d'arène consommé aux métadonnées du rapport d'audit pour tracer la mémoire.
89. Exposer dans le rapport JSON le nombre de types et de règles effectivement analysés.
90. Journaliser sur stderr le motif exact de chaque échec d'allocation d'arène.
91. Ajouter le décompte par sévérité à la fin du rapport texte comme dans la synthèse Markdown.
92. Émettre un verdict TAMIS_DEAD_RULE pour les règles dont le type source n'est jamais atteint.
93. Réserver la matrice de minimisation hors de l'arène pour ne pas saturer l'espace disponible.
94. Mettre en cache les chemins déjà calculés pour accélérer la minimisation gloutonne sur les gros graphes.
95. Ajouter un banc de mesure comparant le temps du BFS avant et après indexation d'adjacence.
96. Étendre le fuzz à l'API publique avec des entrées tronquées et des tampons de taille nulle.
97. Ajouter un test de non-régression figeant la sortie complète d'une politique de référence.
98. Ajouter un test différentiel sur le parsing des politiques, et pas seulement sur le rendu.
99. Ajouter un mutant qui supprime la détection de directive inconnue dans le parseur.
100. Mesurer la couverture de lignes du cœur et exiger un seuil minimal dans la CI.
