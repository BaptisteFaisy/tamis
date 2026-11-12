<!-- SPDX-License-Identifier: MIT -->
# Idées de consolidation — documentation, codelab et internationalisation (100)

1. Écrire le chapitre 2 « Comprendre le modèle de graphe » avec huit questions de compréhension et un schéma.
2. Écrire le chapitre 3 « Bâtir une politique de zéro », ligne par ligne, depuis un type unique.
3. Écrire le chapitre 4 « Passer en intégration continue », avec un fichier YAML GitHub Actions exécutable.
4. Scinder le codelab en trois parcours : découverte, audit complet, intégration continue, avec objectifs distincts.
5. Ajouter à chaque étape du codelab une commande de contrôle affichant le verdict attendu.
6. Créer docs/codelab/solutions.md, corrigés détaillés des exercices ajoutés en fin de codelab.
7. Ajouter dix exercices gradués à la fin du codelab, du niveau « lu » au niveau « crocodile ».
8. Proposer un exercice final « auditez cette politique », piégée, contenant trois erreurs distinctes.
9. Proposer dans le codelab un mode « express » de cinq minutes, avec les seules étapes indispensables.
10. Insérer dans le codelab une capture du graphe `view` pour chaque étape clé, légende comprise.
11. Ajouter au codelab un encadré « prérequis » listant les quatre commandes réellement utilisées.
12. Écrire un chapitre 0 « Qu'est-ce qu'une politique QNX ? » lisible en trois minutes.
13. Proposer un parcours « sans compilateur » du codelab, rejoué dans le playground du site.
14. Ajouter avant chaque étape un encadré « ce que cet outil ne prouve pas » pour rester honnête.
15. Rendre chaque bloc de terminal copiable via un bouton, avec la sortie réelle du binaire.
16. Publier une version imprimable du codelab, avec sommaire, titres numérotés et code couleur stable.
17. Écrire le glossaire secpol dans docs/GLOSSAIRE-SECPOL.md, une entrée par terme QNX, en français et anglais.
18. Citer dans chaque entrée du glossaire la page QNX source et son niveau de confiance.
19. Séparer dans le glossaire les termes du vrai secpol de ceux du format provisoire de Tamis.
20. Ajouter au glossaire une section « faux amis » : allow/permit, type/class, ability/permission.
21. Documenter dans le glossaire chaque ability QNX rencontrée, avec une règle d'exemple.
22. Expliquer dans le glossaire la différence entre plage nommée et plage littérale, avec contre-exemple.
23. Développer les acronymes SDP, BSP, IFS et SMP dans le glossaire, avec expansion et rôle.
24. Relier chaque entrée du glossaire aux ancres correspondantes du codelab, par lien cliquable.
25. Ajouter au glossaire un index des termes qu'un juge non-QNX risque de mal comprendre.
26. Écrire docs/TUTO-GRAMMAIRE.md, tutoriel pas à pas de la syntaxe secpol en dix pages.
27. Ajouter au tutoriel un chapitre « première règle écrite », avec le résultat exact attendu.
28. Découper GRAMMAR.md en guide didactique court et référence exhaustive annotée par sourçage.
29. Illustrer chaque section du tutoriel par un extrait réellement accepté par le parseur Tamis.
30. Documenter dans le tutoriel les messages d'erreur du parseur, du plus courant au plus rare.
31. Ajouter au tutoriel un chapitre « du format provisoire au vrai secpol », avec tableau des écarts.
32. Écrire dans le tutoriel un exemple complet, chaque ligne commentée en français et en anglais.
33. Publier dans le tutoriel la table des treize verdicts, reliant chaque faute de syntaxe au verdict.
34. Ajouter au tutoriel un « banc de syntaxe » : dix fichiers mini, chacun isolant une construction.
35. Expliquer dans le tutoriel pourquoi le parseur travaille en sous-ensemble strict, avec la décision D-004.
36. Écrire docs/CLI-FR.md : les huit sous-commandes, options, codes de sortie et exemples vérifiés.
37. Écrire docs/CLI-EN.md comme traduction fidèle, avec les mêmes exemples et les mêmes sorties.
38. Générer la référence CLI depuis l'aide du binaire, pour éviter toute divergence avec le code.
39. Ajouter en tête de la référence CLI un tableau « besoin → commande → exemple ».
40. Documenter chaque option avec sa valeur par défaut et son effet observable sur la sortie.
41. Réunir dans une annexe les treize codes de verdict et les trois codes de sortie de la CLI.
42. Publier une cheatsheet CLI d'une page A4, en français et en anglais, en Markdown.
43. Ajouter à la référence CLI des sorties JSON complètes, tronquées et en cas d'erreur.
44. Préciser dans la référence CLI quelles commandes acceptent un chemin distant et lesquelles exigent un fichier.
45. Faire relire la référence CLI par un lecteur sans expérience QNX, et consigner chaque confusion.
46. Écrire docs/MODELE-GRAPHE.md, cours de quinze minutes : nœuds, arêtes, sens de parcours.
47. Ajouter au cours un schéma pas à pas de la propagation d'un droit le long d'une chaîne.
48. Expliquer dans le cours la différence entre arête de règle et arête d'accessibilité transitive.
49. Illustrer dans le cours un cas où le chemin le plus court n'est pas le plus dangereux.
50. Proposer dans le cours un exercice où le lecteur prédit la sortie de `reach` avant de l'exécuter.
51. Montrer dans le cours la transformation d'une politique vers son graphe, étape par étape.
52. Expliquer dans le cours ce qu'est un cycle et pourquoi il rend le typage non trivial.
53. Documenter dans le cours le lien entre `minimize` et le graphe résultant, avec capture d'écran.
54. Faire relire le cours de graphe par un étudiant en sécurité sans formation QNX préalable.
55. Écrire docs/FAQ.md, vingt-cinq questions classées du plus général au plus technique, avec liens.
56. Répondre dans la FAQ à « sans licence QNX, que se passe-t-il ? » avec le repli exact.
57. Répondre dans la FAQ à « pourquoi ne pas remplacer secpolcompile ? » en citant la doc QNX.
58. Ajouter dans la FAQ une section « questions du jury » : douze questions probables, réponses courtes.
59. Documenter dans la FAQ la lecture d'un rapport d'audit Markdown, champ par champ.
60. Expliquer dans la FAQ la différence entre avertissement, erreur et information, et leur code de sortie.
61. Recenser dans la FAQ les limites connues et les cas non couverts, sans euphémisme.
62. Répondre dans la FAQ à « combien de vérifications passent vos tests ? » avec la commande exacte.
63. Ajouter dans la FAQ une section « dépannage » : les cinq pannes d'installation les plus fréquentes.
64. Écrire docs/VERDICTS.md, une fiche par verdict : symptôme, cause, correction, code de sortie.
65. Ajouter à chaque fiche un mini-exemple de politique fautive et la sortie CLI correspondante.
66. Grouper les treize verdicts par famille : typage, accessibilité, redondance, divergence.
67. Indiquer dans chaque fiche l'exigence TAM associée, pour la traçabilité du dossier.
68. Documenter dans le glossaire des verdicts le seuil exact qui distingue un warning d'une error.
69. Ajouter au glossaire une section « faux positifs » : quand un verdict peut rester informatif.
70. Publier le glossaire des verdicts en bilingue, avec les messages du cœur C cités verbatim.
71. Relier chaque verdict du glossaire à la section du site qui l'illustre, par ancre.
72. Extraire toutes les chaînes du site dans content.json, avec clés stables et valeurs FR/EN.
73. Écrire un test qui échoue si une locale FR ou EN manque une clé de content.json.
74. Traduire le codelab en anglais intégralement, en conservant les sorties de terminal identiques.
75. Traduire le glossaire secpol en anglais, en laissant les termes QNX non traduits.
76. Traduire la FAQ en anglais, puis contrôler la parité des questions par un script.
77. Rendre le sélecteur de langue accessible au clavier, avec annonce du changement par aria-live.
78. Persister la langue choisie dans un cookie de session, pour partager un lien traduit.
79. Traduire les messages du cœur C en anglais, ou documenter pourquoi la sortie reste française.
80. Publier une page EN du README, avec la même structure et les mêmes chiffres vérifiés.
81. Traquer dans les pages anglaises les faux amis techniques français, puis les corriger.
82. Ajouter au site un lien d'évitement « Aller au contenu » et des points de repère ARIA.
83. Vérifier le contraste des couleurs du site à 4,5:1 et corriger les valeurs trop faibles.
84. Rendre les onglets du site navigables aux flèches, avec attribut role="tablist".
85. Ajouter au site un contour de focus visible et tester la navigation clavier complète.
86. Transcrire la vidéo de démonstration en texte, puis l'intégrer à l'onglet Preuves.
87. Ajouter un bouton « copier » sur chaque bloc de commande du codelab, sur le site.
88. Déclarer le `lang` de chaque bloc bilingue et tester la page avec un lecteur d'écran.
89. Publier un rapport d'accessibilité du site, listant les violations restantes et leur statut.
90. Adapter le playground aux écrans de mobile et de tablette, sans perte de fonction.
91. Réécrire le README en deux langues, avec un encadré « jugement en 60 secondes » en tête.
92. Documenter l'architecture dans docs/ARCHITECTURE.md : couches, dépendances, flux de données.
93. Ajouter au README un tableau « question → commande → preuve » reliant chaque promesse à un test.
94. Écrire CONTRIBUTING.md : build, style de commits, revue attendue, délai de réponse.
95. Ouvrir le README par un tableau des huit onglets du site, avec leur contenu et leur temps de lecture.
96. Documenter dans le README comment lancer le playground sans compiler le cœur C.
97. Insérer dans le README un schéma d'architecture SVG, lisible directement sur GitHub.
98. Créer docs/PLAN-LECTURE-JURY.md, un parcours de lecture conseillé de dix minutes.
99. Écrire docs/index.md classant chaque document par lecteur cible : juge, contributeur, auditeur.
100. Écrire docs/GLOSSAIRE-PREUVE.md en bilingue : non-interférence, transitivité, contre-exemple, certificat.
