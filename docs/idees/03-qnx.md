<!-- SPDX-License-Identifier: MIT -->
# Idées de consolidation — spécificité QNX (100)

1. Extraire les 66 noms d'abilities du binaire 7.1 et classer chacun root, non-root ou les deux.
2. Signaler toute règle dont l'ability n'existe que dans le jeu root, en nommant le type concerné.
3. Modéliser `inherit` et `no-inherit` entre types, et propager le jeu d'abilities hérité.
4. Mesurer l'écart de privilège entre les jeux root et non-root d'un même type.
5. Établir la table de correspondance des abilities 7.1 vers 8.0, avec statut inconnu par défaut.
6. Vérifier que chaque ability custom dynamique porte un préfixe connu, et refuser les préfixes inventés.
7. Agréger les familles dynamiques (`fsevmgr/`, `vfs/`, `smmu/`) en nœuds de synthèse dans le graphe.
8. Repérer les règles accordant une famille entière d'abilities là où une seule suffirait.
9. Corréler chaque ability QNX avec l'opération système qu'elle autorise réellement, dans un tableau unique.
10. Publier le décompte des abilities accordées par type, et signaler les trois plus larges.
11. Marquer `path_trust`, `untrusted_exec` et `prot_write_and_exec` comme abilities sensibles par construction.
12. Dédier une fixture par famille d'ability, pour tester la sémantique root et non-root.
13. Générer un rapport « abilities jamais révoquées », par type et par ability.
14. Modéliser `able_create` et `xprocess_able` comme délégation de capacité inter-processus, distincte des abilities statiques.
15. Repérer les élévations de privilège rendues possibles par une ability de délégation.

16. Analyser le fichier `ability:range_name:syspage:/…` des plages nommées et vérifier chaque entrée.
17. Bloquer l'analyse dès qu'une plage nommée reste irrésolue, en nommant la plage fautive.
18. Calculer union et intersection des sous-plages par ability, et signaler les recouvrements redondants.
19. Reproduire la fusion des sous-plages contiguës de `secpolcompile` et vérifier l'égalité du résultat.
20. Détecter les plages disjointes qu'un nom unique devrait couvrir, source d'erreur de saisie.
21. Tracer pour chaque plage l'ability, le type et le processus qui l'utilisent.
22. Modéliser l'échec de résolution au boot comme un verdict bloquant, distinct des simples avertissements.
23. Vérifier qu'un intervalle de plage reste dans les bornes numériques admises par son ability.
24. Produire le tableau des plages avec leur résolution effective mesurée sur le banc QNX réel.
25. Comparer les plages fusionnées par `secpolcompile` et celles non fusionnées par `secpolpush`, et signaler l'écart.
26. Simuler le comportement du boot quand une plage manque, sans redémarrer la machine virtuelle.
27. Générer automatiquement le fichier de plages depuis les intervalles déclarés dans la politique.

28. Distinguer dans le graphe les types de canaux des types de processus, avec deux familles de nœuds.
29. Vérifier que toute règle `allow` cite un canal déclaré, et refuser les canaux implicites.
30. Mapper les permissions de chaque classe custom vers les appels `secpol_get_permission` documentés.
31. Signaler les classes custom déclarées mais inutilisées, et les classes utilisées non déclarées.
32. Modéliser le typage d'un objet non-canal par class, avec ses effets sur l'accessibilité.
33. Mesurer l'angle mort d'analyse dû aux objets hors canal : mémoire partagée, connexions, sockets.
34. Produire un rapport par classe custom : permissions demandées, octets lus, contrôles observés.
35. Vérifier que `public_channel` n'est pas accordée à un type plus large que nécessaire.
36. Calculer l'expansion des jokers `*` et `/...` de `allow_attach` vers les chemins réels du banc.
37. Étendre le jumeau JavaScript au tranchement des classes custom, à l'identique du cœur C.
38. Distinguer `allow_link` et `allow_attach` par deux arêtes de graphe aux sémantiques différentes.
39. Modéliser `derive_type` comme dérivation conditionnelle de type, cumulable entre règles.
40. Détecter les cycles `derive_type` et `settypeid` atteignables depuis un type non privilégié.

41. Écrire un client `libsecpol` qui interroge les capacités d'un processus sur la cible réelle.
42. Écrire un client `libsecpolev` qui rejoue un refus et capture son code d'erreur exact.
43. Rattacher chaque code de refus `libsecpolev` à l'ability QNX qui l'a déclenchée.
44. Interroger `/dev/secpolgenerate/unused` sur la cible pour lister les abilities jamais utilisées.
45. Énumérer les messages d'erreur de `secpolcompile` pour en dériver un glossaire de grammaire.
46. Automatiser l'export du rapport d'audit depuis la cible QNX, via le pont SSH du banc.
47. Publier une table stable des codes de refus QNX, avec leur signification documentée.
48. Auditer les appels `secpol_check_permission` côté resource manager, pour chaque classe custom.
49. Vérifier qu'aucun chemin d'accessibilité ne traverse une classe custom non analysée.
50. Comparer l'empreinte de `libsecpol.so.1` de la cible avec la version 8.0 attendue.
51. Instrumenter un client QNX de test pour journaliser chaque décision de sécurité prise.
52. Couvrir chaque code de refus QNX connu par une fixture portant le code attendu.

53. Exécuter `secpolgenerate` en mode diff sur deux politiques et comparer à `tamis diff`.
54. Vérifier que le diff `secpolgenerate` et le diff sémantique signalent le même élargissement de privilège.
55. Rejouer un `secpolpush` sur le banc et vérifier l'effet runtime observé des règles poussées.
56. Simuler `secpolpush` hors cible, en calculant l'état effectif des règles après fusion.
57. Tester le refus de `secpolpush` sur une politique dont une plage nommée reste irrésolue.
58. Documenter la chaîne `secpol.txt` → `secpolcompile` → `secpol.bin` → `secpolpush`, avec l'empreinte de chaque étape.
59. Accepter dans `tamis diff` le format natif produit par `secpolgenerate`.
60. Ajouter la cible `secpol.bin` au harnais oracle, dès l'obtention du SDP.
61. Écrire un scénario de restauration : politique poussée, puis retour à la précédente, sur le banc.
62. Mesurer l'écart exact entre `secpolgenerate` et `secpolcompile` sur un même texte source.

63. Vérifier sur le banc la sémantique réelle de l'option `procnto -T`, et la documenter.
64. Modéliser le pathtrust comme propriété de type, distincte d'une ability accordée.
65. Analyser une politique QNX et signaler les binaires fiables dépourvus de signature attendue.
66. Lister les `path_trust` accordés à des répertoires inscriptibles par d'autres types.
67. Refuser dans une même règle le couple contradictoire `path_trust` et `untrusted_exec`.
68. Cartographier les fonctions fortifiées QNX et vérifier leur présence dans le binaire analysé.
69. Signaler les appels de fonctions fortifiées absents du binaire, donc non protégés.
70. Vérifier qu'un type sans `prot_write_and_exec` n'expose aucun segment mémoire writable et exécutable.
71. Analyser les fichiers ELF du banc pour lister, par binaire, les besoins de pathtrust.
72. Fournir un rapport « pathtrust » reliant chaque type aux binaires qu'il doit pouvoir lancer.
73. Tester sur le banc l'effet de `procnto -T` sur le lancement d'un binaire non fiable.

74. Construire un tableau de divergence 7.1 → 8.0 : abilities, modificateurs, terminaisons, formes de règles.
75. Marquer chaque élément attesté en 7.1 comme « à revérifier en 8.0 », jusqu'à preuve contraire.
76. Écrire un script de revalidation rejouant toutes les attestations 7.1 sur une cible 8.0.
77. Exiger, pour toute affirmation QNX, une citation de sortie du banc dans le dossier de preuve.
78. Enregistrer les sorties brutes du banc dans `preuve/`, avec l'empreinte de la session correspondante.
79. Épingler l'empreinte SHA-256 de l'image du banc dans la documentation de preuve.
80. Documenter la procédure de reproduction du banc : commande QEMU exacte, image, port SSH.
81. Automatiser la collecte : une seule commande capture un lot d'assertions QNX rejouable.
82. Ajouter au banc une politique minimaliste réelle, pour valider la chaîne de compilation.
83. Scripter le démarrage et l'arrêt propre du banc, avec un état publié sur la page du site.
84. Comparer `secpol` 7.1 et `secpol` 8.0 sur les mêmes options, et lister les options apparues.
85. Déduire des messages du visualiseur `secpol` des contraintes de grammaire à vérifier.
86. Vérifier que les seize tableaux du blob `secpol.bin` restent stables entre 7.1 et 8.0.
87. Recouper l'inventaire binaire des abilities avec `abilities.tsv` par test de non-régression automatique.
88. Publier un tableau « ce que 7.1 autorise et 8.0 refuse », dès l'obtention du SDP.

89. Construire `security_matrix` : pour chaque propriété QNX, la politique qui l'exerce et son verdict.
90. Mapper chaque ability QNX vers la propriété de sécurité qu'elle active dans la matrice.
91. Exporter `security_matrix` en CSV, avec une colonne de niveau de preuve par propriété.
92. Générer la matrice directement depuis le modèle, sans intervention manuelle, et vérifier par le différentiel.
93. Documenter l'artefact de preuve QNX : faits attestés, portée exacte, et ce qu'il n'établit pas.
94. Versionner l'artefact de preuve et relier chaque fait à la commande qui l'a produit.
95. Publier dans le README un tableau d'écart 7.1 → 8.0, avec le niveau de confiance par ligne.
96. Lister les questions ouvertes sur QNX 8.0, avec la commande qui y répondra le jour du SDP.
97. Ajouter au site une page « Spécificité QNX » recensant abilities, plages et types réellement supportés.
98. Valider dans le playground du site les abilities QNX, en distinguant les variantes root et non-root.
99. Étiqueter dans le rapport d'audit chaque capacité comme QNX-spécifique, native ou hors périmètre.
100. Fermer chaque marqueur « à vérifier » du relevé de grammaire dès qu'une preuve QNX le confirme.
