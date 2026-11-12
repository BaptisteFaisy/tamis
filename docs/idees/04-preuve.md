<!-- SPDX-License-Identifier: MIT -->
# Idées de consolidation — preuve et vérification formelle (100)

1. Encoder le graphe d'accessibilité en relation SMT-LIB2 et vérifier l'absence de chemin par Z3.
2. Exporter un script SMT-LIB2 rejouable par tiers avec `z3 -smt2` sur chaque paire de types.
3. Traduire chaque règle allow/deny en clauses SMT typées par ability, avec position source.
4. Exiger un modèle Z3 vide pour certifier l'inaccessibilité, et publier le statut UNSAT.
5. Formuler l'équivalence de minimisation comme formule UNSAT de contre-exemple, résolue hors ligne.
6. Encoder les plages nommées en intervalles SMT et prouver leur disjonction et leur couverture.
7. Modéliser settypeid et gain_priv par des relations SMT, puis vérifier l'absence d'élévation.
8. Extraire les unsat cores Z3 pour nommer la règle responsable de chaque contradiction détectée.
9. Vérifier les certificats SMT avec deux solveurs indépendants (Z3, cvc5) et signaler les désaccords.
10. Distinguer SAT, UNSAT et INCONNU dans le rapport de preuve, avec timeout borné par requête.
11. Traduire chaque verdict de preuve en obligation quantifiée, puis joindre la formule SMT correspondante.
12. Publier un paquet SMT-LIB2 signé, rejouable hors ligne par un auditeur sans Tamis.
13. Écrire un vérificateur de certificats indépendant, sans réutiliser le BFS du cœur C.
14. Définir un format de certificat JSON versionné, avec graphe, options et compteurs.
15. Ajouter `tamis proof --verify` pour revalider un certificat contre la politique d'origine.
16. Hacher le graphe, les options et le certificat dans une empreinte SHA-256 unique.
17. Séparer producteur et vérificateur en deux binaires, pour que la preuve ne se valide pas elle-même.
18. Rejouer un certificat arête par arête, en revalidant chaque règle contre sa ligne source.
19. Détecter les certificats frauduleux qui omettent des nœuds, en comparant au graphe complet.
20. Vérifier que le compteur de visites du certificat correspond exactement au parcours exhaustif attendu.
21. Figer le format de preuve dans CONTRACT.md et le tester par un test de non-régression byte-à-byte.
22. Émettre un contre-certificat négatif contenant le chemin d'attaque rejouable et ses règles fautives.
23. Encoder le certificat comme terme de preuve acceptant un vérificateur tiers de type Lean.
24. Signer les certificats avec Ed25519, et vérifier la signature hors ligne par un script shell.
25. Inscrire l'empreinte du binaire vérificateur dans le certificat, pour lier preuve et outil.
26. Rejeter tout certificat produit par une version de Tamis absente du registre des empreintes connues.
27. Reproduire les certificats bit pour bit, et échouer si deux exécutions divergent d'un octet.
28. Publier dans chaque certificat l'empreinte de la politique auditée et l'identifiant de version.
29. Émettre un rapport d'attestation SLSA : sources, build reproductible, artefacts et empreintes.
30. Signer les attestations avec cosign dans la CI, et vérifier leur provenance au déploiement.
31. Publier un SBOM signé listant la version exacte de Z3 utilisée dans chaque preuve.
32. Reproduire l'analyse dans une image conteneur minimale et comparer les empreintes de certificats.
33. Épingler les versions des outils dans un fichier de verrou, et échouer si Z3 change.
34. Rendre le build déterministe via SOURCE_DATE_EPOCH, et le prouver par deux builds comparés.
35. Vérifier les artefacts publiés par leur hash avant installation, via une commande dédiée.
36. Comparer les empreintes de certificats sur x86, ARM et WSL, pour exiger la reproductibilité multi-plateforme.
37. Détecter la non-reproductibilité par mutant de l'ordre des règles dans les tests.
38. Enregistrer les certificats dans un registre public append-only horodaté, avec preuve d'inclusion.
39. Fournir une commande unique de reproduction : graine, options, hash attendu.
40. Rejouer le contre-exemple sur le QNX 7.1 du banc r630 et consigner le refus réel observé.
41. Générer un script shell rejouant l'élévation de privilège, exécutable tel quel sur cible QNX.
42. Comparer le verdict statique prédit au comportement runtime mesuré, et trancher les désaccords.
43. Fabriquer un mutant de politique cassant le contre-exemple, pour tester la robustesse du détecteur.
44. Rejouer le contre-exemple après application du correctif minimal, en exigeant la mort du chemin.
45. Exécuter le contre-exemple embarqué dans le playground WASM, sans installation ni QNX.
46. Publier un catalogue des vingt contre-exemples les plus parlants, avec script de rejeu.
47. Enregistrer la trace du contre-exemple en vidéo commentée, pour la démonstration du hackathon.
48. Vérifier automatiquement que chaque chemin émis se rejoue via la seule CLI, sans API interne.
49. Cartographier chaque verdict vers les obligations ISO 26262, avec ASIL cible et exigence citée.
50. Générer un tableau de conformité ISO 26262 directement depuis les verdicts d'une politique.
51. Cartographier les propriétés prouvées vers IEC 62304 et EN 50128, avec niveau de criticité.
52. Rattacher les preuves aux exigences UNECE R155 et ISO/SAE 21434, avec identifiants traçables.
53. Doter chaque verdict d'un identifiant d'exigence normatif, stable entre deux versions de Tamis.
54. Produire un dossier de preuve de conformité HTML, imprimable et prêt pour l'auditeur.
55. Relier chaque obligation normative à un identifiant de propriété effectivement prouvée.
56. Mesurer le taux de couverture des exigences normatives par les propriétés automatiquement prouvées.
57. Signaler dans le rapport les exigences qui restent sans propriété prouvée.
58. Publier une matrice exigence→propriété→preuve→certificat→empreinte, exportable en CSV et JSON.
59. Formaliser en Lean 4 la sémantique d'une règle, puis prouver la correction du moteur de chemin.
60. Prouver dans Lean que le BFS visite tous les nœuds atteignables, établissant l'exhaustivité du parcours.
61. Traduire le modèle C en Lean, puis vérifier l'égalité sémantique par reflexivity.
62. Écrire un noyau de vérification minimal, seul artefact dont la correction est assumée publiquement.
63. Formuler la correction de la minimisation comme lemme rejouable dans Coq.
64. Extraire les certificats UNSAT du solveur SMT en termes vérifiables par un tiers.
65. Réduire la base de confiance au noyau de preuve, en documentant tout ce qui n'est pas formel.
66. Publier les sources formelles avec leurs empreintes, pour permettre une relecture indépendante.
67. Employer Isabelle/HOL pour prouver l'ordre et la couverture des intervalles de plages nommées.
68. Distinguer dans le rapport ce qui est formellement prouvé, testé, ou seulement heuristique.
69. Attacher un niveau de confiance à chaque verdict, de l'observation directe à la preuve formelle.
70. Afficher un score de preuve : part des paires de types dont la non-interférence est établie.
71. Publier un tableau de bord HTML montrant la progression de la preuve par type.
72. Classer publiquement les politiques les mieux prouvées, avec leur score et leur empreinte.
73. Récompenser le contre-exemple le plus court soumis chaque semaine, avec un classement gamifié.
74. Attribuer un badge par propriété : prouvée, contre-exemple, inconnue ou hors-budget.
75. Afficher dans le rapport Markdown le compte exact de paires non-interférentes sur le total.
76. Publier un « mur des preuves » : chaque certificat avec son hash, son statut et son auteur.
77. Rejouer la BFS pas à pas dans le navigateur, en visualisant la preuve se construire.
78. Rendre le pourcentage de graphe exploré visible par preuve, pour quantifier la couverture.
79. Distinguer « aucun chemin trouvé » de « budget dépassé » dans le code de sortie de la CLI.
80. Déclarer dans le certificat les limites soundness : temps, mémoire, nombre de règles explorées.
81. Détecter les graphes explosifs (fork exponentiel) et les signaler dans le certificat.
82. Refuser d'émettre un certificat de non-interférence si une règle est marquée non déterministe.
83. Exposer les limites du modèle de preuve dans une annexe du rapport, sans surprise d'audit.
84. Fournir une checklist d'auto-évaluation du vérificateur de certificats, à passer avant publication.
85. Faire relire le format de certificat par un tiers indépendant, et publier le compte rendu.
86. Vérifier le certificat avec une réimplémentation Python indépendante, comme le jumeau C.
87. Comparer les compteurs de visites du cœur C et du jumeau Python, comme preuve de parcours exhaustif.
88. Tracer la correspondance entre chaque arête du certificat et la règle qui l'a créée.
89. Recalculer le hash du certificat après relecture par le vérificateur, et tracer les deux valeurs.
90. Dater chaque preuve avec une horloge monotone, en distinguant horodatage et durée d'exécution.
91. Exécuter deux analyses concurrentes sur la même politique, et exiger des certificats identiques.
92. Consigner le profil de compilation dans le certificat, pour exclure toute dérive d'optimizer.
93. Comparer le certificat obtenu avec et sans optimisations, et exiger des verdicts identiques.
94. Horodater chaque preuve et expirer les certificats dépassant la durée de validité choisie.
95. Documenter dans le rapport d'audit les hypothèses exactes de la preuve de non-interférence.
96. Comparer la preuve au diff sémantique, pour signaler toute divergence entre verdict et différence.
97. Extraire de la preuve une section « pourquoi c'est sûr » en langage non technique.
98. Mesurer le délai entre publication d'une politique et obtention du certificat de non-interférence.
99. Implémenter la vérification incrémentale : re-prouver uniquement les paires touchées par un diff.
100. Proposer un challenge de reproduction : retrouver la politique à partir d'un certificat publié.
