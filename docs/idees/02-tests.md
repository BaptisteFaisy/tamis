<!-- SPDX-License-Identifier: MIT -->
# Idées de consolidation — tests et vérification (100)

1. Mesurer la couverture de lignes par module avec gcov, et publier le rapport lcov en artefact CI.
2. Fixer un seuil de couverture global à 85 %, bloquant toute pull request qui le fait reculer.
3. Exiger 100 % de couverture de branches dans core/src/proof.c et core/src/reach.c.
4. Couvrir chaque code de diagnostic émis par core/src/lint.c par un test dédié.
5. Publier un badge de couverture dans le README, rafraîchi automatiquement par le job de tests.
6. Lister dans un rapport dédié les lignes jamais exécutées, avec le nom du module responsable.
7. Commenter automatiquement sur chaque pull request le delta de couverture face à la branche de base.
8. Instrumenter le jumeau Python avec coverage.py et viser 90 % de host/tamis_report.py.
9. Tester explicitement chaque code de sortie de la CLI (0, 1, 2) et documenter leur signification.
10. Documenter dans le dépôt chaque exclusion de couverture, avec sa justification et sa date.
11. Suivre la couverture par condition de compilation, et tester explicitement chaque variante.
12. Générer des politiques valides aléatoires par graine, et exiger que reach et proof concordent.
13. Vérifier par propriété que le diff est involutif : inverser A vers B et B vers A renverse les verdicts.
14. Vérifier par propriété que minimiser une politique déjà minimale ne retire plus aucune règle.
15. Vérifier par propriété qu'ajouter une règle autorisante ne réduit jamais le nombre de chemins trouvés.
16. Vérifier par propriété que permuter les règles d'une politique ne change aucun verdict.
17. Appliquer Hypothesis au parseur de modèle, avec rétrécissement automatique du plus petit contre-exemple.
18. Vérifier par propriété que l'empreinte SHA-256 du rapport change dès qu'un seul verdict change.
19. Vérifier par propriété qu'une politique sans aucune règle ne produit ni chemin ni erreur.
20. Étendre le fuzz au générateur de verdicts, et exiger que chaque sortie reste bien formée.
21. Vérifier par propriété que le nombre de nœuds visités par la BFS reste borné par la taille du graphe.
22. Renommer tous les types d'une politique, et exiger des verdicts renommés mais de même code.
23. Copier une fixture sous un autre nom, et exiger un rapport strictement identique.
24. Réordonner les lignes d'une politique, et exiger un rapport identique octet pour octet.
25. Fusionner deux politiques sans conflit, et exiger la conservation des verdicts de la première.
26. Convertir un rapport en JSON puis Markdown, et exiger le même nombre de verdicts listés.
27. Insérer une règle inatteignable, et exiger un verdict identique signalé comme redondant.
28. Comparer deux politiques sémantiquement équivalentes, et exiger le même ensemble de chemins minimaux.
29. Remplacer un deny par un allow, et exiger que diff signale un élargissement de privilège.
30. Inverser source et destination d'une requête, et exiger un chemin symétrique de même longueur.
31. Compiler les tests avec ThreadSanitizer, et exiger l'absence de course détectée.
32. Exécuter les binaires de test en parallèle avec xargs -P, et exiger le même compte de vérifications.
33. Passer la CLI sous valgrind helgrind, pour détecter les accès concurrents non protégés.
34. Vérifier que deux écritures concurrentes dans le même rapport ne s'entrelacent pas.
35. Tester le pont local sous requêtes HTTP simultanées, et exiger des réponses cohérentes et sans mélange.
36. Mesurer le débit du pont sous cinquante requêtes parallèles, et fixer un plancher documenté.
37. Forcer le passage du ramasse-miettes Python pendant le rendu, et exiger un ordre de verdicts stable.
38. Réduire le délai maximal par test à dix secondes, et échouer au premier dépassement.
39. Compiler le cœur en 32 bits, et rejouer toute la suite pour traquer les casts tronqués.
40. Comparer les sérialisations JSON produites en 32 et 64 bits, octet pour octet.
41. Activer les sanitizers undefined et integer, et exiger zéro diagnostic sur la suite complète.
42. Exécuter le rendu Markdown sous locale C et sous locale UTF-8, et exiger la même sortie.
43. Ajouter un comparateur de repli sur l'identifiant de règle, pour stabiliser l'ordre des tris.
44. Compiler pour big-endian sous QEMU, et comparer les verdicts avec la cible little-endian.
45. Ajouter macOS et Alpine à la matrice CI, et exiger zéro divergence de rapport.
46. Vérifier que la lecture binaire préserve les fins de ligne CRLF des fixtures.
47. Épingler gcc et clang dans la CI, et échouer si un compilateur change un octet de sortie.
48. Vérifier que les compteurs de visites ne débordent pas sur 32 bits, avec 100 000 règles.
49. Stocker les rapports attendus de chaque fixture dans tests/golden/, versionnés avec le code.
50. Comparer chaque golden au résultat courant, et afficher un diff lisible dès le premier écart.
51. Enregistrer l'empreinte SHA-256 de chaque golden, pour détecter toute retouche accidentelle du dépôt.
52. Interdire toute régénération automatique des goldens en CI, et l'exiger en commit dédié.
53. Comparer le schéma JSON des verdicts, et échouer sur toute clé ajoutée ou retirée.
54. Exécuter le rendu sur mille politiques générées, et exiger une sortie strictement stable.
55. Consigner dans un changelog dédié chaque régression de verdict corrigée.
56. Exiger qu'un nouveau code de verdict soit toujours accompagné d'un golden daté.
57. Relancer la suite trois fois par graine, et échouer si un test s'avère instable.
58. Épingler les graines du différentiel dans un fichier versionné, et les parcourir en matrice CI.
59. Randomiser l'ordre d'exécution des binaires de test, et exiger un résultat identique à chaque fois.
60. Détecter les tests dépendant de l'heure système ou du fuseau horaire, en les exécutant en UTC.
61. Vérifier que les fichiers temporaires sont nettoyés, y compris quand un test échoue au milieu.
62. Mesurer la durée de chaque binaire de test, et alerter au-delà de soixante secondes.
63. Exécuter la suite dans un conteneur éphémère, pour éliminer tout état résiduel du poste.
64. Détecter les tests dépendant du répertoire courant, en les lançant depuis deux répertoires.
65. Mettre en cache les objets de compilation ccache, avec invalidation par empreinte des sources.
66. Découper la suite C en quatre jobs parallèles par module, pour tenir le budget de cinq minutes.
67. Ajouter une matrice macOS et Alpine sur le seul job de tests unitaires, pour limiter le coût.
68. Porter le fuzz à 200 000 entrées en campagne nocturne, hors du budget des pull requests.
69. Épingler les actions GitHub par SHA, pour qu'une mise à jour amont ne casse pas la CI.
70. Publier un commentaire automatique de pull request avec couverture, durée et nombre de vérifications.
71. Réutiliser un artefact de cache entre jobs, pour ne pas reconstruire le cœur C deux fois.
72. Échouer la CI si le nombre de vérifications vertes diminue face à la branche de base.
73. Isoler les mutants dans un job dédié, déclenché sur la branche principale et chaque nuit.
74. Exécuter le job pages en parallèle du job core, sans dépendance entre eux.
75. Tester le rendu de chaque onglet du site avec le mini-DOM, y compris les onglets bilingues.
76. Vérifier que le site fonctionne hors ligne, avec tous les assets servis localement.
77. Auditer l'accessibilité du site : labels associés, contrastes vérifiés, navigation au clavier.
78. Tester que le playground du site refuse une politique dépassant la taille maximale autorisée.
79. Mesurer le temps de chargement du site, et échouer au-delà de deux secondes.
80. Comparer le rapport affiché par le site au rapport de la CLI, pour une même politique.
81. Vérifier que chaque lien du formulaire Devpost pointe vers une ressource réellement disponible.
82. Exécuter le site dans un vrai navigateur headless, et non plus seulement dans le mini-DOM.
83. Tester le comportement du site quand la fenêtre est redimensionnée ou que le JavaScript échoue.
84. Vérifier que le pont refuse toute origine non locale, avec un test par schéma d'Origin.
85. Tester l'expiration et l'impréguabilité du jeton, par essais de force brute bornés.
86. Vérifier que le pont n'écoute que sur 127.0.0.1, et jamais sur toutes les interfaces.
87. Tester le rejet des requêtes sans Content-Type JSON, et des corps malformés ou tronqués.
88. Vérifier que le pont interrompt proprement un tamis enfant qui dépasse le délai imparti.
89. Vérifier que les réponses du pont ne divulguent ni chemin local ni version du serveur.
90. Tester le refus d'une politique dépassant la taille maximale, et le code de retour exact.
91. Vérifier que le pont survit à une exception Python, journalise l'erreur et rend la main.
92. Tester qnx-vm.sh avec un faux binaire qemu, simulant démarrage, arrêt et échec.
93. Vérifier que le rapport d'acceptation du banc est complet, et que le banc sort en code zéro.
94. Tester le téléchargement OCI contre un registre simulé, couvrant le jeton expiré.
95. Vérifier que le banc détecte l'absence de KVM et bascule proprement en émulation TCG.
96. Vérifier que l'échec d'une vérification arrête quand même l'invité, sans laisser de processus orphelin.
97. Tester l'API contre les méthodes HTTP interdites, les corps tronqués et les en-têtes dupliqués.
98. Vérifier que l'API ne renvoie ni trace Python ni version du serveur en cas d'erreur.
99. Soumettre le pont à des injections de commande via la politique, et exiger un refus systématique.
100. Vérifier qu'aucune entrée utilisateur n'atteint une ligne de commande du shell, par test automatisé.
