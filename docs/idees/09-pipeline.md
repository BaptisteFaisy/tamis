<!-- SPDX-License-Identifier: MIT -->
# Idées de consolidation — pipeline, CI et sécurité de l'outillage (100)

1. Limiter le débit du pont à soixante requêtes par minute, et renvoyer 429 au-delà.
2. Expirer automatiquement la session du pont après trente minutes d'inactivité, puis arrêter le service.
3. Révoquer le jeton à la demande par un signal, sans redémarrage du pont ni perte du banc.
4. Journaliser chaque requête du pont en JSON : méthode, chemin, statut, durée, identifiant de session.
5. Ne jamais journaliser le jeton, les politiques reçues, ni le corps complet des requêtes.
6. Taguer chaque refus par un motif normalisé (401, 403, 429), exploitable dans un tableau de bord.
7. Vérifier l'en-tête Host pour bloquer le DNS rebinding contre 127.0.0.1, sauf hôte local exact.
8. Ouvrir une seconde socket UNIX en mode 0600, pour éviter toute exposition du pont au réseau TCP.
9. Stocker le jeton du pont dans un fichier 0600, et ne l'afficher qu'une seule fois au démarrage.
10. Exécuter le pont sous un compte dédié, sans droit d'écriture hors du répertoire de travail.
11. Écrire les politiques dans un fichier temporaire privé, ouvert en O_EXCL, puis supprimé à la fin.
12. Vérifier l'empreinte SHA-256 du binaire core C avant chaque invocation faite par le pont.
13. Plafonner le pont à quatre analyses simultanées, et mettre les requêtes excédentaires en file.
14. Interrompre toute analyse dépassant le délai imparti, et tuer le processus enfant sans zombie.
15. Valider le corps JSON reçu avec un schéma strict, et refuser toute clé inattendue.
16. Refuser toute valeur de champ texte contenant un chemin absolu ou une séquence de traversée.
17. Refuser toute politique dépassant 512 Kio, et indiquer au client la taille maximale acceptée.
18. Exiger une confirmation explicite du client avant tout démarrage ou arrêt du banc QNX.
19. Verrouiller le banc avec un fichier d'exclusion, pour interdire deux démonstrations concurrentes.
20. Exiger un nonce unique par requête du pont, et rejeter toute requête rejouée.
21. Enregistrer l'empreinte SHA-256 de chaque politique analysée, dans un journal d'audit séparé.
22. Épingler la clé SSH du banc dans un fichier 0600, et vérifier l'hôte avant toute connexion.
23. Désactiver StrictHostKeyChecking et le forwarding d'agent sur la session SSH du banc.
24. Nettoyer au démarrage les fichiers temporaires et les processus enfants laissés par un pont précédent.
25. Vérifier qu'aucun processus enfant du pont ne survit à l'arrêt du serveur HTTP.
26. Stocker le jeton côté navigateur dans sessionStorage, jamais dans localStorage ni dans l'URL.
27. Ne jamais transmettre le jeton du pont dans une URL, un paramètre de requête ou un fragment.
28. Servir aussi le pont en HTTPS local avec un certificat éphémère, pour chiffrer le jeton en transit.
29. Exposer un endpoint /journal restreint au jeton, listant les vingt derniers appels sans leur contenu.
30. Refuser toute commande distante du banc absente de la liste blanche des préréglages figés.
31. Fixer un budget de durée par job dans un fichier ci-budget.yml, et échouer si un job le dépasse.
32. Programmer l'échec du pipeline dès qu'une étape dépasse le double de sa durée médiane.
33. Restreindre le GITHUB_TOKEN au strict nécessaire : lecture seule par défaut, écriture seulement pour pages.
34. Interdire les workflows pull_request_target, qui exposent les secrets aux pull requests externes.
35. Bloquer la CI si un secret est détecté dans l'historique complet du dépôt par gitleaks.
36. Exiger deux relecteurs distincts sur toute modification de reach.c, proof.c ou diff.c.
37. Exiger le statut ci réussi comme condition obligatoire de fusion sur la branche principale.
38. Protéger la branche principale par ruleset : interdiction du force-push et de la suppression.
39. Programmer la fusion automatique des correctifs Dependabot, seulement après passage de la CI.
40. Programmer l'installation de la toolchain depuis un fichier versionné, pour ne pas dépendre de l'image runner.
41. Désactiver le cache sur le job mutants, pour garantir une reconstruction à froid de chaque mutant.
42. Conserver les artefacts de job sept jours, et les purger automatiquement au-delà.
43. Mesurer la couverture du jumeau JavaScript comparée au cœur C, et publier l'écart résiduel.
44. Joindre au run un rapport détaillant chaque mutation et le test qui la tue.
45. Rejouer en priorité les mutants du diff de la pull request, avant la campagne complète du core.
46. Vérifier que le site régénéré est identique au site commité, et échouer sur toute dérive.
47. Valider la syntaxe de tous les workflows YAML avec actionlint, avant le lancement du premier job.
48. Tester les workflows localement avec act, sur un push de branche, avant d'ouvrir la pull request.
49. Reconstruire le site dans un répertoire temporaire, puis comparer l'artefact publié au dépôt.
50. Exécuter le déploiement pages seulement si le job de tests a réussi sur le même commit.
51. Publier un manifeste SHA256SUMS de tous les artefacts, et le vérifier avant tout téléchargement.
52. Épingler les images de runner et les conteneurs par empreinte, jamais par étiquette flottante.
53. Plafonner la taille des artefacts uploadés par le job de couverture, pour tenir le quota GitHub.
54. Reconstruire le cœur deux fois dans le même job, et exiger des binaires identiques octet pour octet.
55. Comparer le coût en minutes de la CI au budget hebdomadaire, et alerter dès qu'il est dépassé.
56. Publier une GitHub Release automatique au tag v*, contenant le binaire et les notes de version.
57. Refuser de publier si le tag ne correspond pas à la version déclarée dans CONTRACT.md.
58. Générer les notes de version automatiquement, à partir des commits conventionnels du dépôt.
59. Signer le tag de release avec GPG, et vérifier la signature avant toute publication.
60. Publier le binaire dans un paquet Debian et un paquet RPM minimal, construits depuis la même source.
61. Distribuer le pont dans une image OCI non-root publiée sur GHCR, avec le binaire C embarqué.
62. Épingler le pont et le site sur une version commune, et tester leur compatibilité dans la CI.
63. Épingler la version du pont dans le schéma du protocole, et refuser un client incompatible.
64. Joindre au tag de release un fichier SHA256SUMS, vérifié automatiquement par un job dédié.
65. Publier le binaire dans une archive versionnée contenant la licence, le NOTICE et le contrat.
66. Épingler la version de QEMU utilisée par le banc dans un fichier de verrou versionné du dépôt.
67. Décrire l'image QNX comme non redistribuable dans l'artefact de release, sans jamais l'inclure.
68. Joindre au tag de release le rapport d'acceptation du banc, avec la date de mesure.
69. Auditer la liste des actions GitHub utilisées, et justifier chacune dans un fichier de revue.
70. Graver la version de build dans la sortie de la CLI, pour tracer précisément l'artefact exécuté.
71. Rendre chaque nom d'artefact auto-descriptif : version, commit et empreinte courte.
72. Consigner dans chaque artefact le numéro de version, le commit source et la date de build.
73. Rendre le manifeste des artefacts append-only, et refuser toute retouche d'une entrée publiée.
74. Vérifier que le binaire publié ne contient aucune chaîne de chemin absolu de build.
75. Vérifier que le dépôt n'importe aucun module Python hors bibliothèque standard.
76. Construire le binaire de release dans un conteneur épinglé, et publier l'empreinte de son image.
77. Interdire toute modification du schéma JSON sans incrémenter le numéro de version du contrat.
78. Comparer l'empreinte du binaire publié à celle du job de test du même commit.
79. Publier la commande unique de reconstruction, et la faire vérifier par un job dédié.
80. Reconstruire le binaire depuis la release dans un conteneur vierge, et exiger la même empreinte.
81. Sauvegarder un instantané du disque QNX avant chaque session, puis le restaurer systématiquement ensuite.
82. Rejouer hors ligne les sessions SSH enregistrées, pour valider l'harnais sans démarrer la VM.
83. Consigner dans un journal d'audit horodaté chaque commande exécutée sur l'invité QNX.
84. Arrêter automatiquement la VM après quinze minutes d'inactivité, via un minuteur systemd utilisateur.
85. Surveiller la santé du banc par un watchdog, et redémarrer QEMU si l'invité ne répond plus.
86. Surveiller l'espace disque du préfixe QEMU, et purger les instantanés dépassant cinq jours.
87. Monter l'image QNX en lecture seule, et n'écrire que dans un overlay jetable.
88. Isoler le réseau de l'invité : pas de passerelle, pas de DNS sortant, seulement SSH local.
89. Faire expirer les sessions du banc après une heure, avec déconnexion puis arrêt de l'invité.
90. Interdire toute commande du pont déclenchant l'arrêt de la VM sans confirmation explicite du client.
91. Programmer une alerte de maintenance quand l'image QNX du banc dépasse six mois d'ancienneté.
92. Vérifier l'intégrité de l'image QNX par empreinte avant chaque démarrage, et refuser tout écart.
93. Vérifier en CI que chaque fichier du dépôt porte un en-tête SPDX, et l'exiger pour les nouveaux.
94. Maintenir un fichier NOTICE listant les licences tierces et leurs obligations de redistribution.
95. Bloquer la CI si un fichier de plus de dix mégaoctets apparaît, pour interdire l'image QNX.
96. Interdire la publication de contenu QNX non redistribuable dans le site comme dans les artefacts.
97. Publier une SECURITY.md : procédure de signalement, délai de réponse et versions affectées.
98. Ajouter au dossier de soumission une clause de licence QNX, sans citer le contenu du SDP.
99. Exiger la signature des commits pour toute contribution externe, contrôlée par la CI.
100. Nommer un responsable par fichier d'outillage, et le consigner dans le fichier CODEOWNERS.
