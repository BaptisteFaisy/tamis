<!-- SPDX-License-Identifier: MIT -->
# 1000 idées de consolidation — catalogue

**1000 idées** réparties en **10 thèmes**, sans doublon.
Généré par `tools/merge_idees.py` à partir des lots `docs/idees/*.md`.

## Sommaire

- **cœur C et robustesse** — 100 idées
- **tests et vérification** — 100 idées
- **spécificité QNX** — 100 idées
- **preuve et vérification formelle** — 100 idées
- **performance et échelle** — 100 idées
- **expérience développeur, CLI et distribution** — 100 idées
- **documentation, codelab et internationalisation** — 100 idées
- **démo, vidéo, juges et soumission** — 100 idées
- **pipeline, CI et sécurité de l'outillage** — 100 idées
- **process, pilotage et finalisation** — 100 idées

## cœur C et robustesse

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

## tests et vérification

101. Mesurer la couverture de lignes par module avec gcov, et publier le rapport lcov en artefact CI.
102. Fixer un seuil de couverture global à 85 %, bloquant toute pull request qui le fait reculer.
103. Exiger 100 % de couverture de branches dans core/src/proof.c et core/src/reach.c.
104. Couvrir chaque code de diagnostic émis par core/src/lint.c par un test dédié.
105. Publier un badge de couverture dans le README, rafraîchi automatiquement par le job de tests.
106. Lister dans un rapport dédié les lignes jamais exécutées, avec le nom du module responsable.
107. Commenter automatiquement sur chaque pull request le delta de couverture face à la branche de base.
108. Instrumenter le jumeau Python avec coverage.py et viser 90 % de host/tamis_report.py.
109. Tester explicitement chaque code de sortie de la CLI (0, 1, 2) et documenter leur signification.
110. Documenter dans le dépôt chaque exclusion de couverture, avec sa justification et sa date.
111. Suivre la couverture par condition de compilation, et tester explicitement chaque variante.
112. Générer des politiques valides aléatoires par graine, et exiger que reach et proof concordent.
113. Vérifier par propriété que le diff est involutif : inverser A vers B et B vers A renverse les verdicts.
114. Vérifier par propriété que minimiser une politique déjà minimale ne retire plus aucune règle.
115. Vérifier par propriété qu'ajouter une règle autorisante ne réduit jamais le nombre de chemins trouvés.
116. Vérifier par propriété que permuter les règles d'une politique ne change aucun verdict.
117. Appliquer Hypothesis au parseur de modèle, avec rétrécissement automatique du plus petit contre-exemple.
118. Vérifier par propriété que l'empreinte SHA-256 du rapport change dès qu'un seul verdict change.
119. Vérifier par propriété qu'une politique sans aucune règle ne produit ni chemin ni erreur.
120. Étendre le fuzz au générateur de verdicts, et exiger que chaque sortie reste bien formée.
121. Vérifier par propriété que le nombre de nœuds visités par la BFS reste borné par la taille du graphe.
122. Renommer tous les types d'une politique, et exiger des verdicts renommés mais de même code.
123. Copier une fixture sous un autre nom, et exiger un rapport strictement identique.
124. Réordonner les lignes d'une politique, et exiger un rapport identique octet pour octet.
125. Fusionner deux politiques sans conflit, et exiger la conservation des verdicts de la première.
126. Convertir un rapport en JSON puis Markdown, et exiger le même nombre de verdicts listés.
127. Insérer une règle inatteignable, et exiger un verdict identique signalé comme redondant.
128. Comparer deux politiques sémantiquement équivalentes, et exiger le même ensemble de chemins minimaux.
129. Remplacer un deny par un allow, et exiger que diff signale un élargissement de privilège.
130. Inverser source et destination d'une requête, et exiger un chemin symétrique de même longueur.
131. Compiler les tests avec ThreadSanitizer, et exiger l'absence de course détectée.
132. Exécuter les binaires de test en parallèle avec xargs -P, et exiger le même compte de vérifications.
133. Passer la CLI sous valgrind helgrind, pour détecter les accès concurrents non protégés.
134. Vérifier que deux écritures concurrentes dans le même rapport ne s'entrelacent pas.
135. Tester le pont local sous requêtes HTTP simultanées, et exiger des réponses cohérentes et sans mélange.
136. Mesurer le débit du pont sous cinquante requêtes parallèles, et fixer un plancher documenté.
137. Forcer le passage du ramasse-miettes Python pendant le rendu, et exiger un ordre de verdicts stable.
138. Réduire le délai maximal par test à dix secondes, et échouer au premier dépassement.
139. Compiler le cœur en 32 bits, et rejouer toute la suite pour traquer les casts tronqués.
140. Comparer les sérialisations JSON produites en 32 et 64 bits, octet pour octet.
141. Activer les sanitizers undefined et integer, et exiger zéro diagnostic sur la suite complète.
142. Exécuter le rendu Markdown sous locale C et sous locale UTF-8, et exiger la même sortie.
143. Ajouter un comparateur de repli sur l'identifiant de règle, pour stabiliser l'ordre des tris.
144. Compiler pour big-endian sous QEMU, et comparer les verdicts avec la cible little-endian.
145. Ajouter macOS et Alpine à la matrice CI, et exiger zéro divergence de rapport.
146. Vérifier que la lecture binaire préserve les fins de ligne CRLF des fixtures.
147. Épingler gcc et clang dans la CI, et échouer si un compilateur change un octet de sortie.
148. Vérifier que les compteurs de visites ne débordent pas sur 32 bits, avec 100 000 règles.
149. Stocker les rapports attendus de chaque fixture dans tests/golden/, versionnés avec le code.
150. Comparer chaque golden au résultat courant, et afficher un diff lisible dès le premier écart.
151. Enregistrer l'empreinte SHA-256 de chaque golden, pour détecter toute retouche accidentelle du dépôt.
152. Interdire toute régénération automatique des goldens en CI, et l'exiger en commit dédié.
153. Comparer le schéma JSON des verdicts, et échouer sur toute clé ajoutée ou retirée.
154. Exécuter le rendu sur mille politiques générées, et exiger une sortie strictement stable.
155. Consigner dans un changelog dédié chaque régression de verdict corrigée.
156. Exiger qu'un nouveau code de verdict soit toujours accompagné d'un golden daté.
157. Relancer la suite trois fois par graine, et échouer si un test s'avère instable.
158. Épingler les graines du différentiel dans un fichier versionné, et les parcourir en matrice CI.
159. Randomiser l'ordre d'exécution des binaires de test, et exiger un résultat identique à chaque fois.
160. Détecter les tests dépendant de l'heure système ou du fuseau horaire, en les exécutant en UTC.
161. Vérifier que les fichiers temporaires sont nettoyés, y compris quand un test échoue au milieu.
162. Mesurer la durée de chaque binaire de test, et alerter au-delà de soixante secondes.
163. Exécuter la suite dans un conteneur éphémère, pour éliminer tout état résiduel du poste.
164. Détecter les tests dépendant du répertoire courant, en les lançant depuis deux répertoires.
165. Mettre en cache les objets de compilation ccache, avec invalidation par empreinte des sources.
166. Découper la suite C en quatre jobs parallèles par module, pour tenir le budget de cinq minutes.
167. Ajouter une matrice macOS et Alpine sur le seul job de tests unitaires, pour limiter le coût.
168. Porter le fuzz à 200 000 entrées en campagne nocturne, hors du budget des pull requests.
169. Épingler les actions GitHub par SHA, pour qu'une mise à jour amont ne casse pas la CI.
170. Publier un commentaire automatique de pull request avec couverture, durée et nombre de vérifications.
171. Réutiliser un artefact de cache entre jobs, pour ne pas reconstruire le cœur C deux fois.
172. Échouer la CI si le nombre de vérifications vertes diminue face à la branche de base.
173. Isoler les mutants dans un job dédié, déclenché sur la branche principale et chaque nuit.
174. Exécuter le job pages en parallèle du job core, sans dépendance entre eux.
175. Tester le rendu de chaque onglet du site avec le mini-DOM, y compris les onglets bilingues.
176. Vérifier que le site fonctionne hors ligne, avec tous les assets servis localement.
177. Auditer l'accessibilité du site : labels associés, contrastes vérifiés, navigation au clavier.
178. Tester que le playground du site refuse une politique dépassant la taille maximale autorisée.
179. Mesurer le temps de chargement du site, et échouer au-delà de deux secondes.
180. Comparer le rapport affiché par le site au rapport de la CLI, pour une même politique.
181. Vérifier que chaque lien du formulaire Devpost pointe vers une ressource réellement disponible.
182. Exécuter le site dans un vrai navigateur headless, et non plus seulement dans le mini-DOM.
183. Tester le comportement du site quand la fenêtre est redimensionnée ou que le JavaScript échoue.
184. Vérifier que le pont refuse toute origine non locale, avec un test par schéma d'Origin.
185. Tester l'expiration et l'impréguabilité du jeton, par essais de force brute bornés.
186. Vérifier que le pont n'écoute que sur 127.0.0.1, et jamais sur toutes les interfaces.
187. Tester le rejet des requêtes sans Content-Type JSON, et des corps malformés ou tronqués.
188. Vérifier que le pont interrompt proprement un tamis enfant qui dépasse le délai imparti.
189. Vérifier que les réponses du pont ne divulguent ni chemin local ni version du serveur.
190. Tester le refus d'une politique dépassant la taille maximale, et le code de retour exact.
191. Vérifier que le pont survit à une exception Python, journalise l'erreur et rend la main.
192. Tester qnx-vm.sh avec un faux binaire qemu, simulant démarrage, arrêt et échec.
193. Vérifier que le rapport d'acceptation du banc est complet, et que le banc sort en code zéro.
194. Tester le téléchargement OCI contre un registre simulé, couvrant le jeton expiré.
195. Vérifier que le banc détecte l'absence de KVM et bascule proprement en émulation TCG.
196. Vérifier que l'échec d'une vérification arrête quand même l'invité, sans laisser de processus orphelin.
197. Tester l'API contre les méthodes HTTP interdites, les corps tronqués et les en-têtes dupliqués.
198. Vérifier que l'API ne renvoie ni trace Python ni version du serveur en cas d'erreur.
199. Soumettre le pont à des injections de commande via la politique, et exiger un refus systématique.
200. Vérifier qu'aucune entrée utilisateur n'atteint une ligne de commande du shell, par test automatisé.

## spécificité QNX

201. Extraire les 66 noms d'abilities du binaire 7.1 et classer chacun root, non-root ou les deux.
202. Signaler toute règle dont l'ability n'existe que dans le jeu root, en nommant le type concerné.
203. Modéliser `inherit` et `no-inherit` entre types, et propager le jeu d'abilities hérité.
204. Mesurer l'écart de privilège entre les jeux root et non-root d'un même type.
205. Établir la table de correspondance des abilities 7.1 vers 8.0, avec statut inconnu par défaut.
206. Vérifier que chaque ability custom dynamique porte un préfixe connu, et refuser les préfixes inventés.
207. Agréger les familles dynamiques (`fsevmgr/`, `vfs/`, `smmu/`) en nœuds de synthèse dans le graphe.
208. Repérer les règles accordant une famille entière d'abilities là où une seule suffirait.
209. Corréler chaque ability QNX avec l'opération système qu'elle autorise réellement, dans un tableau unique.
210. Publier le décompte des abilities accordées par type, et signaler les trois plus larges.
211. Marquer `path_trust`, `untrusted_exec` et `prot_write_and_exec` comme abilities sensibles par construction.
212. Dédier une fixture par famille d'ability, pour tester la sémantique root et non-root.
213. Générer un rapport « abilities jamais révoquées », par type et par ability.
214. Modéliser `able_create` et `xprocess_able` comme délégation de capacité inter-processus, distincte des abilities statiques.
215. Repérer les élévations de privilège rendues possibles par une ability de délégation.
216. Analyser le fichier `ability:range_name:syspage:/…` des plages nommées et vérifier chaque entrée.
217. Bloquer l'analyse dès qu'une plage nommée reste irrésolue, en nommant la plage fautive.
218. Calculer union et intersection des sous-plages par ability, et signaler les recouvrements redondants.
219. Reproduire la fusion des sous-plages contiguës de `secpolcompile` et vérifier l'égalité du résultat.
220. Détecter les plages disjointes qu'un nom unique devrait couvrir, source d'erreur de saisie.
221. Tracer pour chaque plage l'ability, le type et le processus qui l'utilisent.
222. Modéliser l'échec de résolution au boot comme un verdict bloquant, distinct des simples avertissements.
223. Vérifier qu'un intervalle de plage reste dans les bornes numériques admises par son ability.
224. Produire le tableau des plages avec leur résolution effective mesurée sur le banc QNX réel.
225. Comparer les plages fusionnées par `secpolcompile` et celles non fusionnées par `secpolpush`, et signaler l'écart.
226. Simuler le comportement du boot quand une plage manque, sans redémarrer la machine virtuelle.
227. Générer automatiquement le fichier de plages depuis les intervalles déclarés dans la politique.
228. Distinguer dans le graphe les types de canaux des types de processus, avec deux familles de nœuds.
229. Vérifier que toute règle `allow` cite un canal déclaré, et refuser les canaux implicites.
230. Mapper les permissions de chaque classe custom vers les appels `secpol_get_permission` documentés.
231. Signaler les classes custom déclarées mais inutilisées, et les classes utilisées non déclarées.
232. Modéliser le typage d'un objet non-canal par class, avec ses effets sur l'accessibilité.
233. Mesurer l'angle mort d'analyse dû aux objets hors canal : mémoire partagée, connexions, sockets.
234. Produire un rapport par classe custom : permissions demandées, octets lus, contrôles observés.
235. Vérifier que `public_channel` n'est pas accordée à un type plus large que nécessaire.
236. Calculer l'expansion des jokers `*` et `/...` de `allow_attach` vers les chemins réels du banc.
237. Étendre le jumeau JavaScript au tranchement des classes custom, à l'identique du cœur C.
238. Distinguer `allow_link` et `allow_attach` par deux arêtes de graphe aux sémantiques différentes.
239. Modéliser `derive_type` comme dérivation conditionnelle de type, cumulable entre règles.
240. Détecter les cycles `derive_type` et `settypeid` atteignables depuis un type non privilégié.
241. Écrire un client `libsecpol` qui interroge les capacités d'un processus sur la cible réelle.
242. Écrire un client `libsecpolev` qui rejoue un refus et capture son code d'erreur exact.
243. Rattacher chaque code de refus `libsecpolev` à l'ability QNX qui l'a déclenchée.
244. Interroger `/dev/secpolgenerate/unused` sur la cible pour lister les abilities jamais utilisées.
245. Énumérer les messages d'erreur de `secpolcompile` pour en dériver un glossaire de grammaire.
246. Automatiser l'export du rapport d'audit depuis la cible QNX, via le pont SSH du banc.
247. Publier une table stable des codes de refus QNX, avec leur signification documentée.
248. Auditer les appels `secpol_check_permission` côté resource manager, pour chaque classe custom.
249. Vérifier qu'aucun chemin d'accessibilité ne traverse une classe custom non analysée.
250. Comparer l'empreinte de `libsecpol.so.1` de la cible avec la version 8.0 attendue.
251. Instrumenter un client QNX de test pour journaliser chaque décision de sécurité prise.
252. Couvrir chaque code de refus QNX connu par une fixture portant le code attendu.
253. Exécuter `secpolgenerate` en mode diff sur deux politiques et comparer à `tamis diff`.
254. Vérifier que le diff `secpolgenerate` et le diff sémantique signalent le même élargissement de privilège.
255. Rejouer un `secpolpush` sur le banc et vérifier l'effet runtime observé des règles poussées.
256. Simuler `secpolpush` hors cible, en calculant l'état effectif des règles après fusion.
257. Tester le refus de `secpolpush` sur une politique dont une plage nommée reste irrésolue.
258. Documenter la chaîne `secpol.txt` → `secpolcompile` → `secpol.bin` → `secpolpush`, avec l'empreinte de chaque étape.
259. Accepter dans `tamis diff` le format natif produit par `secpolgenerate`.
260. Ajouter la cible `secpol.bin` au harnais oracle, dès l'obtention du SDP.
261. Écrire un scénario de restauration : politique poussée, puis retour à la précédente, sur le banc.
262. Mesurer l'écart exact entre `secpolgenerate` et `secpolcompile` sur un même texte source.
263. Vérifier sur le banc la sémantique réelle de l'option `procnto -T`, et la documenter.
264. Modéliser le pathtrust comme propriété de type, distincte d'une ability accordée.
265. Analyser une politique QNX et signaler les binaires fiables dépourvus de signature attendue.
266. Lister les `path_trust` accordés à des répertoires inscriptibles par d'autres types.
267. Refuser dans une même règle le couple contradictoire `path_trust` et `untrusted_exec`.
268. Cartographier les fonctions fortifiées QNX et vérifier leur présence dans le binaire analysé.
269. Signaler les appels de fonctions fortifiées absents du binaire, donc non protégés.
270. Vérifier qu'un type sans `prot_write_and_exec` n'expose aucun segment mémoire writable et exécutable.
271. Analyser les fichiers ELF du banc pour lister, par binaire, les besoins de pathtrust.
272. Fournir un rapport « pathtrust » reliant chaque type aux binaires qu'il doit pouvoir lancer.
273. Tester sur le banc l'effet de `procnto -T` sur le lancement d'un binaire non fiable.
274. Construire un tableau de divergence 7.1 → 8.0 : abilities, modificateurs, terminaisons, formes de règles.
275. Marquer chaque élément attesté en 7.1 comme « à revérifier en 8.0 », jusqu'à preuve contraire.
276. Écrire un script de revalidation rejouant toutes les attestations 7.1 sur une cible 8.0.
277. Justifier toute affirmation QNX par une citation de sortie du banc, dans le dossier de preuve.
278. Enregistrer les sorties brutes du banc dans `preuve/`, avec l'empreinte de la session correspondante.
279. Épingler l'empreinte SHA-256 de l'image du banc dans la documentation de preuve.
280. Documenter la procédure de reproduction du banc : commande QEMU exacte, image, port SSH.
281. Automatiser la collecte : une seule commande capture un lot d'assertions QNX rejouable.
282. Ajouter au banc une politique minimaliste réelle, pour valider la chaîne de compilation.
283. Scripter le démarrage et l'arrêt propre du banc, avec un état publié sur la page du site.
284. Comparer `secpol` 7.1 et `secpol` 8.0 sur les mêmes options, et lister les options apparues.
285. Déduire des messages du visualiseur `secpol` des contraintes de grammaire à vérifier.
286. Vérifier que les seize tableaux du blob `secpol.bin` restent stables entre 7.1 et 8.0.
287. Recouper l'inventaire binaire des abilities avec `abilities.tsv` par test de non-régression automatique.
288. Publier un tableau « ce que 7.1 autorise et 8.0 refuse », dès l'obtention du SDP.
289. Construire `security_matrix` : pour chaque propriété QNX, la politique qui l'exerce et son verdict.
290. Mapper chaque ability QNX vers la propriété de sécurité qu'elle active dans la matrice.
291. Exporter `security_matrix` en CSV, avec une colonne de niveau de preuve par propriété.
292. Générer la matrice directement depuis le modèle, sans intervention manuelle, et vérifier par le différentiel.
293. Documenter l'artefact de preuve QNX : faits attestés, portée exacte, et ce qu'il n'établit pas.
294. Versionner l'artefact de preuve et relier chaque fait à la commande qui l'a produit.
295. Publier dans le README un tableau d'écart 7.1 → 8.0, avec le niveau de confiance par ligne.
296. Lister les questions ouvertes sur QNX 8.0, avec la commande qui y répondra le jour du SDP.
297. Ajouter au site une page « Spécificité QNX » recensant abilities, plages et types réellement supportés.
298. Valider dans le playground du site les abilities QNX, en distinguant les variantes root et non-root.
299. Étiqueter dans le rapport d'audit chaque capacité comme QNX-spécifique, native ou hors périmètre.
300. Fermer chaque marqueur « à vérifier » du relevé de grammaire dès qu'une preuve QNX le confirme.

## preuve et vérification formelle

301. Encoder le graphe d'accessibilité en relation SMT-LIB2 et vérifier l'absence de chemin par Z3.
302. Exporter un script SMT-LIB2 rejouable par tiers avec `z3 -smt2` sur chaque paire de types.
303. Traduire chaque règle allow/deny en clauses SMT typées par ability, avec position source.
304. Exiger un modèle Z3 vide pour certifier l'inaccessibilité, et publier le statut UNSAT.
305. Formuler l'équivalence de minimisation comme formule UNSAT de contre-exemple, résolue hors ligne.
306. Encoder les plages nommées en intervalles SMT et prouver leur disjonction et leur couverture.
307. Modéliser settypeid et gain_priv par des relations SMT, puis vérifier l'absence d'élévation.
308. Extraire les unsat cores Z3 pour nommer la règle responsable de chaque contradiction détectée.
309. Vérifier les certificats SMT avec deux solveurs indépendants (Z3, cvc5) et signaler les désaccords.
310. Distinguer SAT, UNSAT et INCONNU dans le rapport de preuve, avec timeout borné par requête.
311. Traduire chaque verdict de preuve en obligation quantifiée, puis joindre la formule SMT correspondante.
312. Publier un paquet SMT-LIB2 signé, rejouable hors ligne par un auditeur sans Tamis.
313. Écrire un vérificateur de certificats indépendant, sans réutiliser le BFS du cœur C.
314. Définir un format de certificat JSON versionné, avec graphe, options et compteurs.
315. Ajouter `tamis proof --verify` pour revalider un certificat contre la politique d'origine.
316. Hacher le graphe, les options et le certificat dans une empreinte SHA-256 unique.
317. Séparer producteur et vérificateur en deux binaires, pour que la preuve ne se valide pas elle-même.
318. Rejouer un certificat arête par arête, en revalidant chaque règle contre sa ligne source.
319. Détecter les certificats frauduleux qui omettent des nœuds, en comparant au graphe complet.
320. Vérifier que le compteur de visites du certificat correspond exactement au parcours exhaustif attendu.
321. Figer le format de preuve dans CONTRACT.md et le tester par un test de non-régression byte-à-byte.
322. Émettre un contre-certificat négatif contenant le chemin d'attaque rejouable et ses règles fautives.
323. Encoder le certificat comme terme de preuve acceptant un vérificateur tiers de type Lean.
324. Signer les certificats avec Ed25519, et vérifier la signature hors ligne par un script shell.
325. Inscrire l'empreinte du binaire vérificateur dans le certificat, pour lier preuve et outil.
326. Rejeter tout certificat produit par une version de Tamis absente du registre des empreintes connues.
327. Reproduire les certificats bit pour bit, et échouer si deux exécutions divergent d'un octet.
328. Publier dans chaque certificat l'empreinte de la politique auditée et l'identifiant de version.
329. Émettre un rapport d'attestation SLSA : sources, build reproductible, artefacts et empreintes.
330. Signer les attestations avec cosign dans la CI, et vérifier leur provenance au déploiement.
331. Publier un SBOM signé listant la version exacte de Z3 utilisée dans chaque preuve.
332. Reproduire l'analyse dans une image conteneur minimale et comparer les empreintes de certificats.
333. Épingler les versions des outils dans un fichier de verrou, et échouer si Z3 change.
334. Rendre le build déterministe via SOURCE_DATE_EPOCH, et le prouver par deux builds comparés.
335. Vérifier les artefacts publiés par leur hash avant installation, via une commande dédiée.
336. Comparer les empreintes de certificats sur x86, ARM et WSL, pour exiger la reproductibilité multi-plateforme.
337. Détecter la non-reproductibilité par mutant de l'ordre des règles dans les tests.
338. Enregistrer les certificats dans un registre public append-only horodaté, avec preuve d'inclusion.
339. Fournir une commande unique de reproduction : graine, options, hash attendu.
340. Rejouer le contre-exemple sur le QNX 7.1 du banc r630 et consigner le refus réel observé.
341. Générer un script shell rejouant l'élévation de privilège, exécutable tel quel sur cible QNX.
342. Comparer le verdict statique prédit au comportement runtime mesuré, et trancher les désaccords.
343. Fabriquer un mutant de politique cassant le contre-exemple, pour tester la robustesse du détecteur.
344. Rejouer le contre-exemple après application du correctif minimal, en exigeant la mort du chemin.
345. Exécuter le contre-exemple embarqué dans le playground WASM, sans installation ni QNX.
346. Publier un catalogue des vingt contre-exemples les plus parlants, avec script de rejeu.
347. Enregistrer la trace du contre-exemple en vidéo commentée, pour la démonstration du hackathon.
348. Vérifier automatiquement que chaque chemin émis se rejoue via la seule CLI, sans API interne.
349. Cartographier chaque verdict vers les obligations ISO 26262, avec ASIL cible et exigence citée.
350. Générer un tableau de conformité ISO 26262 directement depuis les verdicts d'une politique.
351. Cartographier les propriétés prouvées vers IEC 62304 et EN 50128, avec niveau de criticité.
352. Rattacher les preuves aux exigences UNECE R155 et ISO/SAE 21434, avec identifiants traçables.
353. Doter chaque verdict d'un identifiant d'exigence normatif, stable entre deux versions de Tamis.
354. Produire un dossier de preuve de conformité HTML, imprimable et prêt pour l'auditeur.
355. Relier chaque obligation normative à un identifiant de propriété effectivement prouvée.
356. Mesurer le taux de couverture des exigences normatives par les propriétés automatiquement prouvées.
357. Signaler dans le rapport les exigences qui restent sans propriété prouvée.
358. Publier une matrice exigence→propriété→preuve→certificat→empreinte, exportable en CSV et JSON.
359. Formaliser en Lean 4 la sémantique d'une règle, puis prouver la correction du moteur de chemin.
360. Prouver dans Lean que le BFS visite tous les nœuds atteignables, établissant l'exhaustivité du parcours.
361. Traduire le modèle C en Lean, puis vérifier l'égalité sémantique par reflexivity.
362. Écrire un noyau de vérification minimal, seul artefact dont la correction est assumée publiquement.
363. Formuler la correction de la minimisation comme lemme rejouable dans Coq.
364. Extraire les certificats UNSAT du solveur SMT en termes vérifiables par un tiers.
365. Réduire la base de confiance au noyau de preuve, en documentant tout ce qui n'est pas formel.
366. Publier les sources formelles avec leurs empreintes, pour permettre une relecture indépendante.
367. Employer Isabelle/HOL pour prouver l'ordre et la couverture des intervalles de plages nommées.
368. Distinguer dans le rapport ce qui est formellement prouvé, testé, ou seulement heuristique.
369. Attacher un niveau de confiance à chaque verdict, de l'observation directe à la preuve formelle.
370. Afficher un score de preuve : part des paires de types dont la non-interférence est établie.
371. Publier un tableau de bord HTML montrant la progression de la preuve par type.
372. Classer publiquement les politiques les mieux prouvées, avec leur score et leur empreinte.
373. Récompenser le contre-exemple le plus court soumis chaque semaine, avec un classement gamifié.
374. Attribuer un badge par propriété : prouvée, contre-exemple, inconnue ou hors-budget.
375. Afficher dans le rapport Markdown le compte exact de paires non-interférentes sur le total.
376. Publier un « mur des preuves » : chaque certificat avec son hash, son statut et son auteur.
377. Rejouer la BFS pas à pas dans le navigateur, en visualisant la preuve se construire.
378. Rendre le pourcentage de graphe exploré visible par preuve, pour quantifier la couverture.
379. Distinguer « aucun chemin trouvé » de « budget dépassé » dans le code de sortie de la CLI.
380. Déclarer dans le certificat les limites soundness : temps, mémoire, nombre de règles explorées.
381. Détecter les graphes explosifs (fork exponentiel) et les signaler dans le certificat.
382. Refuser d'émettre un certificat de non-interférence si une règle est marquée non déterministe.
383. Exposer les limites du modèle de preuve dans une annexe du rapport, sans surprise d'audit.
384. Fournir une checklist d'auto-évaluation du vérificateur de certificats, à passer avant publication.
385. Faire relire le format de certificat par un tiers indépendant, et publier le compte rendu.
386. Vérifier le certificat avec une réimplémentation Python indépendante, comme le jumeau C.
387. Comparer les compteurs de visites du cœur C et du jumeau Python, comme preuve de parcours exhaustif.
388. Tracer la correspondance entre chaque arête du certificat et la règle qui l'a créée.
389. Recalculer le hash du certificat après relecture par le vérificateur, et tracer les deux valeurs.
390. Dater chaque preuve avec une horloge monotone, en distinguant horodatage et durée d'exécution.
391. Exécuter deux analyses concurrentes sur la même politique, et exiger des certificats identiques.
392. Consigner le profil de compilation dans le certificat, pour exclure toute dérive d'optimizer.
393. Comparer le certificat obtenu avec et sans optimisations, et exiger des verdicts identiques.
394. Horodater chaque preuve et expirer les certificats dépassant la durée de validité choisie.
395. Documenter dans le rapport d'audit les hypothèses exactes de la preuve de non-interférence.
396. Comparer la preuve au diff sémantique, pour signaler toute divergence entre verdict et différence.
397. Extraire de la preuve une section « pourquoi c'est sûr » en langage non technique.
398. Mesurer le délai entre publication d'une politique et obtention du certificat de non-interférence.
399. Implémenter la vérification incrémentale : re-prouver uniquement les paires touchées par un diff.
400. Proposer un challenge de reproduction : retrouver la politique à partir d'un certificat publié.

## performance et échelle

401. Remplacer la recherche linéaire des types par une table de hachage ouverte, indexée par nom.
402. Ranger les règles en liste d'adjacence contiguë, pour itérer les successeurs sans balayer tout.
403. Construire un index inverse cible-règles, permettant d'énumérer les prédécesseurs rapidement.
404. Substituer au tableau de visite un jeu de bits de 64 bits, divisant la mémoire par huit.
405. Interner les noms de types et les abilities dans une table unique de chaînes partagées.
406. Remplacer les index entiers signés par des uint32, pour doubler la densité mémoire du graphe.
407. Trier les règles par couple source-cible, pour comparer deux politiques par fusion de listes.
408. Séparer les règles sans ability des règles nommées, pour accélérer la détection de redondance.
409. Précalculer le degré entrant et sortant de chaque type, stocké sur seize bits.
410. Stocker les règles en format CSR à deux passes, pour un parcours contigu des arcs.
411. Effectuer un seul parcours en largeur par source, au lieu d'un parcours par couple demandé.
412. Calculer la fermeture transitive par Warshall sur bitsets, quand le nombre de types reste modéré.
413. Représenter l'ensemble des successeurs d'un type par un bitset de 4096 bits.
414. Réduire le graphe à sa condensation, puis n'analyser que les classes de composantes.
415. Détecter les composantes fortement connexes, pour partager un seul calcul entre leurs membres.
416. Exclure du calcul toute-paire les types dépourvus de toute règle sortante.
417. Écarter les nœuds terminaux du graphe de transition, sans modifier les résultats obtenus.
418. Comparer le modèle source et le modèle minimisé par différence d'arcs, au lieu de toute-paire.
419. Vérifier l'équivalence de minimisation par différence d'atteignables, en O(E log E).
420. Borner l'analyse toute-paire par un budget de paires, et publier la fraction non couverte.
421. Invalider le cache d'atteignabilité par l'empreinte du sous-graphe modifié, non du fichier entier.
422. Déterminer le plus petit ensemble de types affectés par une règle ajoutée ou retirée.
423. Réutiliser la matrice d'atteignabilité précédente, en ne recalculant que les paires concernées.
424. Identifier chaque règle par un index stable, pour localiser une modification sans relire le texte.
425. Répertorier les règles par numéro de ligne, pour comparer deux versions sans réanalyse complète.
426. Mémoïser le résultat d'accessibilité par couple de types, invalidé par version du modèle.
427. Mémoïser le parcours par source, pour réutiliser un seul calcul sur toutes les cibles demandées.
428. Employer un cache LRU de taille fixe pour les requêtes interactives du site.
429. Consigner dans le cache la version du moteur, et le rejeter si elle diffère de l'actuelle.
430. Charger le cache au démarrage de la CLI, pour rejouer une analyse identique en moins de 50 ms.
431. Répartir les paires toute-paire entre threads, chacun détenant son propre tampon de visite.
432. Réutiliser un pool de threads existant, alloué une seule fois au démarrage du processus.
433. Paralléliser le rendu Markdown du rapport, en préservant l'ordre exact des verdicts émis.
434. Analyser plusieurs fichiers en parallèle depuis la CLI, une politique par processus enfant.
435. Protéger l'émission des verdicts par un mutex, en conservant un ordre global stable.
436. Remplacer le verrou global par des opérations atomiques sur les bitsets partagés.
437. Mesurer le gain du parallélisme sur huit cœurs, et publier la courbe temps selon le nombre de threads.
438. N'activer le parallélisme qu'au-delà d'un seuil de types, pour ne pas pénaliser les petits cas.
439. Fixer un plafond d'arène configurable, et échouer proprement dès qu'il est dépassé.
440. Compter les octets demandés par moteur, et publier le pic de mémoire dans le rapport.
441. Réutiliser un pool de tampons entre les parcours, au lieu d'allouer à chaque requête.
442. Réserver l'espace des tampons de BFS une seule fois par analyse, puis le recycler.
443. Construire le modèle à la volée, règle par règle, sans conserver le texte source en mémoire.
444. Réinitialiser l'arène entre deux analyses, avec un pointeur de reprise documenté.
445. Évaluer la taille d'arène requise avant l'analyse, et refuser de démarrer si elle dépasse.
446. Interner les libellés de règles, pour éviter de copier une chaîne par chemin construit.
447. Remplacer les pointeurs ability et id par des index sur seize bits dans la règle.
448. Ajouter une option --budget-temps, qui interrompt l'analyse et publie l'état atteint.
449. Distinguer le mode rapide, limité aux paires demandées, du mode exhaustif toute-paire.
450. Retourner un code de sortie distinct pour budget épuisé, séparé de l'absence de chemin.
451. Afficher le pourcentage de paires couvert avant épuisement du budget, dans le rapport.
452. Découper l'analyse toute-paire en lots, avec reprise possible entre deux lots consécutifs.
453. Sauvegarder l'état d'analyse après chaque lot, pour reprendre après une interruption.
454. Fixer un seuil par défaut de trente secondes, réglable par variable d'environnement.
455. Encoder l'ensemble des types atteignables par un diagramme de décisions binaire.
456. Repérer les sous-graphes isomorphes, pour partager leur représentation compacte.
457. Représenter les capacités par ability sous forme de bitsets, un bit par ability.
458. Grouper les règles par ability, puis détecter les conflits par intersection de bitsets.
459. Comparer deux politiques par différence de bitsets d'abilities, plutôt que par liste de règles.
460. Vectoriser la fermeture transitive par des opérations AND, quand le nombre de types est multiple de 64.
461. Employer les instructions SSE2 pour comparer deux bitsets de 4096 bits d'un seul coup.
462. Compter les bits d'un ensemble d'atteignabilité avec popcount, sans boucle.
463. Aligner les tableaux de bitsets sur soixante-quatre octets, pour le cache et la vectorisation.
464. Dérouler la boucle interne du parcours en blocs, pour supprimer les branches par sommet.
465. Compiler le cœur avec -O3 et -flto, en vérifiant que les sorties restent identiques.
466. Ajouter une cible perf au Makefile, compilant avec -pg pour le profilage gprof.
467. Publier le profil gprof de la toute-paire, annexé au dossier de preuve.
468. Mesurer les instructions et les défauts de cache avec perf stat sur une analyse complète.
469. Publier un tableau de référence des durées, sur trois tailles de corpus documentées.
470. Échouer la CI si une analyse de référence met plus du double de son temps habituel.
471. Réduire le coût d'appel avec -fno-plt et la visibilité restreinte des symboles exportés.
472. Marquer les boucles critiques du parcours comme hot, et mesurer le gain obtenu.
473. Compiler le jumeau JavaScript avec un bundler, et garder un benchmark de référence commun.
474. Exécuter le même benchmark dans le navigateur, et publier le résultat sur le site.
475. Générer des politiques synthétiques de mille à cent mille règles, par graine reproductible.
476. Publier le générateur de corpus synthétique, pour rendre chaque benchmark rejouable par tous.
477. Mesurer la durée d'un parcours selon la densité du graphe, et tracer la courbe obtenue.
478. Tester la pire casse sur une chaîne de dix mille types, qui épuise le parcours en largeur.
479. Tester un graphe dense de mille types et cent mille arcs, pour vérifier la borne mémoire.
480. Comparer les durées du cœur C, du jumeau Python et du jumeau JavaScript sur un même corpus.
481. Analyser une politique directement depuis un flux, sans jamais la charger entière.
482. Écrire le rapport JSON en flux, sans construire la chaîne complète en mémoire.
483. Débiter le rapport par blocs de soixante-quatre kilo-octets, avec vidage progressif.
484. Accepter une politique sur l'entrée standard, pour les chaînes de traitement en CI.
485. Mesurer le débit d'analyse en mode flux, en politiques traitées par seconde.
486. Découper le jumeau JavaScript en modules, pour ne pas recharger tout le script.
487. Exécuter l'analyse dans un Web Worker, pour ne pas figer l'interface du navigateur.
488. Transférer le modèle au Worker par tableau transférable, sans copie de structure.
489. Refuser dans le terrain de jeu toute politique dépassant deux cent kilo-octets.
490. Afficher une barre de progression pendant les analyses longues, côté navigateur.
491. Documenter pourquoi l'accélération GPU a été écartée, avec le coût de transfert mesuré.
492. Comparer le coût du GPU et celui du processeur, et publier la mesure dans le dossier de preuve.
493. Renoncer au GPU pour la toute-paire, le transfert de données dépassant le gain mesuré.
494. Retenir le vectoriel SIMD sur processeur comme seule voie d'accélération retenue.
495. Justifier l'absence de GPU dans l'architecture, par un paragraphe documenté et sourcé.
496. Fournir un démon local indexant les politiques, pour une réanalyse instantanée.
497. Invalidater le cache par numéro de génération, plutôt que par comparaison de contenu.
498. Limiter la taille du cache sur disque, avec éviction du plus ancien utilisé et journal.
499. Vérifier l'égalité des verdicts entre mode exhaustif et mode rapide, sur cent paires tirées.
500. Publier le tableau de montée en échelle, avec les durées mesurées jusqu'à cent mille règles.

## expérience développeur, CLI et distribution

501. Ajouter une sous-commande `scope` qui publie le périmètre de complétude et les angles morts bornés.
502. Ajouter une sous-commande `init` qui génère un squelette de politique commenté et exécutable.
503. Ajouter une sous-commande `bench` qui mesure le temps d'analyse par politique et par moteur.
504. Ajouter une sous-commande `capabilities` qui liste les moteurs, les formats et les codes de sortie.
505. Ajouter un fichier de configuration `tamis.toml` par projet, lu par la CLI.
506. Permettre à la CLI de lire plusieurs fichiers de configuration en cascade, du plus général au plus précis.
507. Valider `tamis.toml` au démarrage et nommer précisément la clé fautive dans le message d'erreur.
508. Rendre chaque message d'erreur de la CLI actionnable, en citant la commande à lancer ensuite.
509. Distinguer les codes de sortie 1, 2, 3 et 4 par un message explicite dans la sortie standard.
510. Ajouter `--quiet` pour n'afficher que le rapport, sans les messages de progression.
511. Ajouter `--verbose` affichant l'accélérateur, la taille de l'arène et les compteurs de visites.
512. Ajouter `--dry-run` à toute commande mutatrice, pour prévisualiser sans rien écrire.
513. Ajouter `--color never|auto|always` pour les sorties capturées dans une CI ou une vidéo.
514. Ajouter `--output <fichier>` pour écrire le rapport sans passer par une redirection shell.
515. Ajouter `--fail-on <sévérité>` pour ne faire échouer la CI qu'à partir d'un seuil choisi.
516. N'échouer sur aucun verdict, et documenter le choix comme une décision de conception assumée.
517. Proposer une complétion bash pour les sept sous-commandes et leurs options.
518. Proposer une complétion zsh générée à partir du même descripteur que l'aide.
519. Générer les pages de manuel à partir de la description des commandes, sans duplication.
520. Faire afficher l'aide en une ligne de commande exécutable quand l'utilisateur se trompe.
521. Ajouter une cible `make install` installant le binaire, la documentation et les exemples.
522. Publier un binaire statique lié musl pour les distributions sans glibc.
523. Publier les versions releases avec une somme SHA-256 et une empreinte cosign par version.
524. Rédiger un guide d'installation par distribution, Ubuntu puis Fedora puis Arch.
525. Fournir une image de conteneur minimale exécutant la CLI sans installation préalable.
526. Publier le paquet dans Homebrew, puis dans les dépôts Nix et Arch.
527. Ajouter un en-tête de version et une sous-commande `version` détaillant la compilation.
528. Compiler avec `-ffile-prefix-map` pour que les chemins de compilation ne fuisent pas dans les symboles.
529. Rendre la compilation déterministe, avec SOURCE_DATE_EPOCH respecté par le Makefile.
530. Publier un SBOM par version, généré depuis la liste des dépendances effectives.
531. Ajouter un installateur qui refuse d'écrire hors d'un préfixe choisi explicitement.
532. Installer les exemples dans un répertoire de données partagées, en lecture seule.
533. Distinguer les données utilisateur (config) des données installées (exemples, schémas).
534. Permettre à la CLI de retrouver ses données via XDG_DATA_DIRS et XDG_CONFIG_HOME.
535. Écrire un test d'installation dans un préfixe jetable, pour garantir que `make install` reste propre.
536. Fournir une action GitHub composite réutilisable, qui construit et exécute Tamis sur une PR.
537. Publier l'action sur le marketplace GitHub, avec un exemple de workflow minimal.
538. Fournir un hook pre-commit qui lance `tamis lint` sur les seules politiques modifiées.
539. Documenter l'installation du hook pre-commit et son désactivation propre.
540. Publier une configuration Neovim qui appelle `tamis lint --format json` sur l'enregistrement.
541. Publier une extension VS Code qui affiche les verdicts en ligne, sous le curseur.
542. Servir un serveur LSP minimal exposant les 13 verdicts sur un document `.secpol`.
543. Compléter le LSP avec la complétion des mots-clés attestés de la grammaire.
544. Fournir un modèle de configuration Neovim et VS Code, partagées entre les deux éditeurs.
545. Publier un modèle GitLab CI qui publie le rapport en artefact de job.
546. Publier un modèle Jenkins qui publie le rapport en archive de build.
547. Publier un modèle CircleCI qui bloque la CI sur un élargissement de privilège.
548. Documenter l'intégration continue avec les cinq plateformes CI les plus courantes.
549. Émettre le rapport au format SARIF, lisible par lesAnalyseurs de sécurité de GitHub.
550. Émettre le rapport au format Checkstyle, lisible par les rapporteurs génériques.
551. Publier le schéma JSON des verdicts dans un fichier versionné, pour validation externe.
552. Tester le schéma JSON contre un corpus de verdicts générés, à chaque version.
553. Versionner le schéma JSON et refuser un rapport produit par une version incompatible.
554. Documenter la politique de compatibilité ascendante du format JSON.
555. Publier les en-têtes C comme une API publique, avec une macro de version.
556. Interdire l'ajout de symboles dans l'espace public sans bumped de version majeure.
557. Fournir des bindings Rustgenerated par bindgen, publiés sur crates.io.
558. Fournir un binding Python Installable par pip, sans dépendance de compilation.
559. Fournir un binding Go minimal, pour l'écosystème cloud.
560. Publier un paquet npm enveloppant le binaire, pour l'usage en CI.
561. Documenter les garanties de stabilité de chaque fonction exportée du cœur.
562. Ajouter un en-tête de dépréciation quiPrevient à la compilation, un an avant le retrait.
563. Retirer les symboles dépréciés avec une release majeure et une note de migration.
564. Écrire un guide de contribution滚.randnneb.editors qui décrit le cycle de vie d'une idée.
565. Ajouter un rapport « coverage by requirement » reliant chaque exigence à ses tests.
566. Publier un rapport de couverture des pages de documentation, à partir des extraits de code.
567. Mesurer le temps de démarrage de la CLI et le garder sous une seconde.
568. Mesurer la taille du binaire et suivre son évolution entre les versions.
569. Proposer un mode « lecture seule » guaranteeing qu'aucun fichier n'est modifié.
570. Vérifier par hachage qu'une analyse donnée ne modifie pas la politique analysée.
571. Permettre de choisir le niveau de détail d'un rapport d'audit via une option.
572. Proposer un rapport HTML autonome, sans dépendance ni requête réseau.
573. Proposer un export CSV des verdicts, pour analyse en tableur.
574. Proposer un export SARIFrévisé, avec les codes d'unicité.
575. Exposer une fonction de bibliothèque unique, `tamis_analyse`, pour les intégrateurs C.
576. Permettre d'enchaîner plusieurs analyses sans recharger la politique en mémoire.
577. Fournir un cache d'analyse indexé par empreinte de la politique, activé par option.
578. Ajouter un cache invalidé automatiquement quand le format du rapport change.
579. Permettre de choisir le niveau de journalisation vers un fichier, sans bruit sur stdout.
580. Rediriger les diagnostics vers stderr, pour que stdout reste un rapport analysable.
581. Garantir qu'un rapport vide reste un rapport valide et analysable, jamais une sortie tronquée.
582. Ajouter un test qui échoue si la sortie standard n'est pas du JSON valide.
583. Ajouter un test qui échoue si la sortie standard contient autre chose que le rapport.
584. Fournir un wrapper `tamis-ci` qui ne sort qu'un booléen, pour les pipelines simples.
585. Documenter le contrat de `tamis-ci` : entrée, sortie, code de retour.
586. Publier un exemple de workflow GitHub bloquant sur un élargissement de privilège.
587. Publier un exemple de workflow GitHub publiant le rapport d'audit en artefact.
588. Écrire un guide « comment mettre Tamis dans une CI » en moins de dix lignes de configuration.
589. Proposer une commande `tamis version --long` listant le commit, la date et les options de compilation.
590. Proposer une commande `tamis doctor` vérifiant l'environnement et signalant les manques.
591. Faire que `tamis doctor` distingue ce qui est bloquant de ce qui est facultatif.
592. Ajouter un test de non-régression vérifiant que la CLI ne grew pas ses options sans documentation.
593. Documenter chaque option de la CLI dans le README et dans l'aide, à partir d'une source unique.
594. Publier une page de « recettes » : filtrer les verdicts, ne garder que les erreurs, etc.
595. Fournir des recettes jq prêtes à l'emploi pour les treize verdicts.
596. Écrire un guide sur la lecture d'un rapport d'audit, destiné à un auditeur non-développeur.
597. Ajouter un exemple de rapport minimal, commenté, dans la documentation.
598. Proposer un « mode narratif » qui explique chaque verdict en une phrase.
599. Écrire un guide de dépannage des erreurs fréquentes de la CLI, indexé par message.
600. Mesurer la satisfaction des utilisateurs du playground et itérer sur les points de friction.

## documentation, codelab et internationalisation

601. Écrire le chapitre 2 « Comprendre le modèle de graphe » avec huit questions de compréhension et un schéma.
602. Écrire le chapitre 3 « Bâtir une politique de zéro », ligne par ligne, depuis un type unique.
603. Écrire le chapitre 4 « Passer en intégration continue », avec un fichier YAML GitHub Actions exécutable.
604. Scinder le codelab en trois parcours : découverte, audit complet, intégration continue, avec objectifs distincts.
605. Ajouter à chaque étape du codelab une commande de contrôle affichant le verdict attendu.
606. Créer docs/codelab/solutions.md, corrigés détaillés des exercices ajoutés en fin de codelab.
607. Ajouter dix exercices gradués à la fin du codelab, du niveau « lu » au niveau « crocodile ».
608. Proposer un exercice final « auditez cette politique », piégée, contenant trois erreurs distinctes.
609. Proposer dans le codelab un mode « express » de cinq minutes, avec les seules étapes indispensables.
610. Insérer dans le codelab une capture du graphe `view` pour chaque étape clé, légende comprise.
611. Ajouter au codelab un encadré « prérequis » listant les quatre commandes réellement utilisées.
612. Écrire un chapitre 0 « Qu'est-ce qu'une politique QNX ? » lisible en trois minutes.
613. Proposer un parcours « sans compilateur » du codelab, rejoué dans le playground du site.
614. Ajouter avant chaque étape un encadré « ce que cet outil ne prouve pas » pour rester honnête.
615. Rendre chaque bloc de terminal copiable via un bouton, avec la sortie réelle du binaire.
616. Publier une version imprimable du codelab, avec sommaire, titres numérotés et code couleur stable.
617. Écrire le glossaire secpol dans docs/GLOSSAIRE-SECPOL.md, une entrée par terme QNX, en français et anglais.
618. Citer dans chaque entrée du glossaire la page QNX source et son niveau de confiance.
619. Séparer dans le glossaire les termes du vrai secpol de ceux du format provisoire de Tamis.
620. Ajouter au glossaire une section « faux amis » : allow/permit, type/class, ability/permission.
621. Documenter dans le glossaire chaque ability QNX rencontrée, avec une règle d'exemple.
622. Expliquer dans le glossaire la différence entre plage nommée et plage littérale, avec contre-exemple.
623. Développer les acronymes SDP, BSP, IFS et SMP dans le glossaire, avec expansion et rôle.
624. Relier chaque entrée du glossaire aux ancres correspondantes du codelab, par lien cliquable.
625. Ajouter au glossaire un index des termes qu'un juge non-QNX risque de mal comprendre.
626. Écrire docs/TUTO-GRAMMAIRE.md, tutoriel pas à pas de la syntaxe secpol en dix pages.
627. Ajouter au tutoriel un chapitre « première règle écrite », avec le résultat exact attendu.
628. Découper GRAMMAR.md en guide didactique court et référence exhaustive annotée par sourçage.
629. Illustrer chaque section du tutoriel par un extrait réellement accepté par le parseur Tamis.
630. Documenter dans le tutoriel les messages d'erreur du parseur, du plus courant au plus rare.
631. Ajouter au tutoriel un chapitre « du format provisoire au vrai secpol », avec tableau des écarts.
632. Écrire dans le tutoriel un exemple complet, chaque ligne commentée en français et en anglais.
633. Publier dans le tutoriel la table des treize verdicts, reliant chaque faute de syntaxe au verdict.
634. Ajouter au tutoriel un « banc de syntaxe » : dix fichiers mini, chacun isolant une construction.
635. Expliquer dans le tutoriel pourquoi le parseur travaille en sous-ensemble strict, avec la décision D-004.
636. Écrire docs/CLI-FR.md : les huit sous-commandes, options, codes de sortie et exemples vérifiés.
637. Écrire docs/CLI-EN.md comme traduction fidèle, avec les mêmes exemples et les mêmes sorties.
638. Générer la référence CLI depuis l'aide du binaire, pour éviter toute divergence avec le code.
639. Ajouter en tête de la référence CLI un tableau « besoin → commande → exemple ».
640. Documenter chaque option avec sa valeur par défaut et son effet observable sur la sortie.
641. Réunir dans une annexe les treize codes de verdict et les trois codes de sortie de la CLI.
642. Publier une cheatsheet CLI d'une page A4, en français et en anglais, en Markdown.
643. Ajouter à la référence CLI des sorties JSON complètes, tronquées et en cas d'erreur.
644. Préciser dans la référence CLI quelles commandes acceptent un chemin distant et lesquelles exigent un fichier.
645. Faire relire la référence CLI par un lecteur sans expérience QNX, et consigner chaque confusion.
646. Écrire docs/MODELE-GRAPHE.md, cours de quinze minutes : nœuds, arêtes, sens de parcours.
647. Ajouter au cours un schéma pas à pas de la propagation d'un droit le long d'une chaîne.
648. Expliquer dans le cours la différence entre arête de règle et arête d'accessibilité transitive.
649. Illustrer dans le cours un cas où le chemin le plus court n'est pas le plus dangereux.
650. Proposer dans le cours un exercice où le lecteur prédit la sortie de `reach` avant de l'exécuter.
651. Montrer dans le cours la transformation d'une politique vers son graphe, étape par étape.
652. Expliquer dans le cours ce qu'est un cycle et pourquoi il rend le typage non trivial.
653. Documenter dans le cours le lien entre `minimize` et le graphe résultant, avec capture d'écran.
654. Faire relire le cours de graphe par un étudiant en sécurité sans formation QNX préalable.
655. Écrire docs/FAQ.md, vingt-cinq questions classées du plus général au plus technique, avec liens.
656. Répondre dans la FAQ à « sans licence QNX, que se passe-t-il ? » avec le repli exact.
657. Répondre dans la FAQ à « pourquoi ne pas remplacer secpolcompile ? » en citant la doc QNX.
658. Ajouter dans la FAQ une section « questions du jury » : douze questions probables, réponses courtes.
659. Documenter dans la FAQ la lecture d'un rapport d'audit Markdown, champ par champ.
660. Expliquer dans la FAQ la différence entre avertissement, erreur et information, et leur code de sortie.
661. Recenser dans la FAQ les limites connues et les cas non couverts, sans euphémisme.
662. Répondre dans la FAQ à « combien de vérifications passent vos tests ? » avec la commande exacte.
663. Ajouter dans la FAQ une section « dépannage » : les cinq pannes d'installation les plus fréquentes.
664. Écrire docs/VERDICTS.md, une fiche par verdict : symptôme, cause, correction, code de sortie.
665. Ajouter à chaque fiche un mini-exemple de politique fautive et la sortie CLI correspondante.
666. Grouper les treize verdicts par famille : typage, accessibilité, redondance, divergence.
667. Indiquer dans chaque fiche l'exigence TAM associée, pour la traçabilité du dossier.
668. Documenter dans le glossaire des verdicts le seuil exact qui distingue un warning d'une error.
669. Ajouter au glossaire une section « faux positifs » : quand un verdict peut rester informatif.
670. Publier le glossaire des verdicts en bilingue, avec les messages du cœur C cités verbatim.
671. Relier chaque verdict du glossaire à la section du site qui l'illustre, par ancre.
672. Extraire toutes les chaînes du site dans content.json, avec clés stables et valeurs FR/EN.
673. Écrire un test qui échoue si une locale FR ou EN manque une clé de content.json.
674. Traduire le codelab en anglais intégralement, en conservant les sorties de terminal identiques.
675. Traduire le glossaire secpol en anglais, en laissant les termes QNX non traduits.
676. Traduire la FAQ en anglais, puis contrôler la parité des questions par un script.
677. Rendre le sélecteur de langue accessible au clavier, avec annonce du changement par aria-live.
678. Persister la langue choisie dans un cookie de session, pour partager un lien traduit.
679. Traduire les messages du cœur C en anglais, ou documenter pourquoi la sortie reste française.
680. Publier une page EN du README, avec la même structure et les mêmes chiffres vérifiés.
681. Traquer dans les pages anglaises les faux amis techniques français, puis les corriger.
682. Ajouter au site un lien d'évitement « Aller au contenu » et des points de repère ARIA.
683. Vérifier le contraste des couleurs du site à 4,5:1 et corriger les valeurs trop faibles.
684. Rendre les onglets du site navigables aux flèches, avec attribut role="tablist".
685. Ajouter au site un contour de focus visible et tester la navigation clavier complète.
686. Transcrire la vidéo de démonstration en texte, puis l'intégrer à l'onglet Preuves.
687. Ajouter un bouton « copier » sur chaque bloc de commande du codelab, sur le site.
688. Déclarer le `lang` de chaque bloc bilingue et tester la page avec un lecteur d'écran.
689. Publier un rapport d'accessibilité du site, listant les violations restantes et leur statut.
690. Adapter le playground aux écrans de mobile et de tablette, sans perte de fonction.
691. Réécrire le README en deux langues, avec un encadré « jugement en 60 secondes » en tête.
692. Documenter l'architecture dans docs/ARCHITECTURE.md : couches, dépendances, flux de données.
693. Ajouter au README un tableau « question → commande → preuve » reliant chaque promesse à un test.
694. Écrire CONTRIBUTING.md : build, style de commits, revue attendue, délai de réponse.
695. Ouvrir le README par un tableau des huit onglets du site, avec leur contenu et leur temps de lecture.
696. Documenter dans le README comment lancer le playground sans compiler le cœur C.
697. Insérer dans le README un schéma d'architecture SVG, lisible directement sur GitHub.
698. Créer docs/PLAN-LECTURE-JURY.md, un parcours de lecture conseillé de dix minutes.
699. Écrire docs/index.md classant chaque document par lecteur cible : juge, contributeur, auditeur.
700. Écrire docs/GLOSSAIRE-PREUVE.md en bilingue : non-interférence, transitivité, contre-exemple, certificat.

## démo, vidéo, juges et soumission

701. Écrire le storyboard de la vidéo en six plans, avec pour chacun la commande exacte affichée à l'écran.
702. Filmer chaque plan séparément avec ffmpeg, pour permettre un montage sans repasser la caméra.
703. Enregistrer la console avec une police à chasse fixe, afin que les colonnes de verdicts restent alignées au montage.
704. Préparer un jeu de policies dédié à la vidéo, distinct des fixtures utilisées par les tests.
705. Programmer les sous-titres au format SRT à partir du script minuté, puis vérifier leur synchronisation au montage.
706. Écrire les sous-titres en deux langues, français et anglais, avec des timecodes strictement identiques.
707. Enregistrer une voix off neutre, dictant les chiffres à voix haute pour l'accessibilité.
708. Choisir une table de mixage à trois niveaux, pour que la narration domine le son du terminal.
709. Réserver les treize premières secondes à un carton-titre sobre, sans logo ni animation coûteuse.
710. Placer le logo Tamis en fond perdu discret, pour ne pas voler l'attention aux verdicts affichés.
711. Prévoir une miniature lisible en 320 pixels de large, avec le chemin d'attaque en rouge.
712. Ajouter des chapitres YouTube marquant chaque plan, pour permettre un clic direct sur la preuve.
713. Vérifier que la vidéo reste compréhensible sans le son, avec les sous-titres activés par défaut.
714. Découper une version courte de 90 secondes, centrée sur la seule preuve de non-interférence, comme repli.
715. Fournir le fichier vidéo dans le dépôt, avec son empreinte SHA-256 et sa durée exacte.
716. Héberger la vidéo sur une ressource accessible sans authentification, puis tester le lien en navigation privée.
717. Monter le plan « preuve » en accéléré quatre fois, pour que l'absence de chemin se lise en deux secondes.
718. Intercaler à l'écran le compteur de nœuds visités pendant la preuve, pour rendre l'exhaustivité visible.
719. Capturer la sortie du banc QNX réel dans un terminal secondaire, reformatée pour tenir dans un tiers de l'écran.
720. Enregistrer le pont local en action, pour montrer le vrai binaire C à côté du jumeau navigateur.
721. Écrire le texte de chaque carton en une phrase de moins de douze mots, lisible sans pause.
722. Tourner une seconde prise de chaque plan, pour remplacer un passage raté sans refaire toute la vidéo.
723. Contrôler que la durée finale s'arrête à 2 min 58, pour rester sous la limite de trois minutes.
724. Enregistrer le visualiseur de graphe en 1080p, sans perte de lisibilité sur les noms de règles.
725. Fournir la transcription texte complète de la narration, en annexe du dossier de soumission.
726. Publier le plan de tournage comme document interne, pour qu'un pair puisse reprendre le tournage.
727. Choisir un fond d'écran neutre pour les plans navigateur, sans barre d'interface visible.
728. Masquer les notifications et le curseur de souris des captures, pour éviter les regards parasites.
729. Rendre le chemin d'attaque clignotant dans la vidéo, via un export vidéo du visualiseur de graphe.
730. Remplacer toute mention de « mock » ou « fake » par le terme exact du contrat, pour ne rien surpromettre.
731. Écrire un script de démonstration de 90 secondes, avec la liste exacte des sept clics du jury.
732. Précharger la politique « attaque » dans le playground, pour que la première vue montre déjà un verdict.
733. Ajouter des liens profonds par onglet, pour ouvrir directement la preuve depuis un billet de réseau social.
734. Figer le jeu de données de démonstration dans une version datée, pour que la démo reste reproductible.
735. Enregistrer une session du banc QNX capturée en direct, comme annexe vidéo de la démonstration.
736. Préparer une capture d'écran du pont affichant le verdict identique du cœur C et du jumeau JavaScript.
737. Tester le pont depuis un second navigateur, pour prouver que le jeton protège bien la session.
738. Documenter la procédure de démarrage du banc en trois commandes, pour la rejouer devant le jury.
739. Mesurer le délai réel entre le clic et l'affichage du verdict, puis publier cette mesure dans le dossier.
740. Vérifier que le playground reste utilisable avec le pont arrêté, sans aucune erreur dans la console.
741. Prévoir un mode hors ligne du playground, avec un court message expliquant l'absence du banc réel.
742. Créer un second jeu de policies de démonstration, plus volumineux, pour tester la tenue en direct.
743. Préparer deux scénarios alternables : le chemin d'attaque trouvé, puis le certificat de non-interférence vert.
744. Écrire un texte de présentation orale de trois minutes, calé sur les sept onglets du site.
745. Répéter la démo avec un collègue jouant le juge sceptique, et consigner chaque objection formulée.
746. Repérer les cinq clics qui produisent le plus d'impact, et les ordonner pour la démonstration orale.
747. Prévoir une version imprimée du parcours du juge, d'une page, à remettre sur simple demande.
748. Placer dans la description du dépôt un lien court et lisible vers la démonstration en ligne.
749. Tester la démonstration sur l'écran d'un téléphone, pour le jury qui consulte le lien en mobilité.
750. Prévoir un mode jury du playground, qui n'affiche que les quatre résultats attendus par les juges.
751. Enregistrer la démonstration complète en vidéo, comme repli si le réseau du jury tombe.
752. Consigner dans le dépôt la configuration exacte du poste de démonstration, avec versions et polices installées.
753. Écrire une phrase de dix mots mémorisant la promesse de Tamis, et la placer en tête de page.
754. Montrer la sortie JSON brute dans un panneau du site, pour prouver que le navigateur n’invente rien.
755. Placer le lien du playground au-dessus de la ligne de flottaison, sur mobile comme sur grand écran.
756. Prévoir un QR code vers la démo, collable sur l’affiche du stand et la diapositive finale.
757. Formuler le problème en une phrase, en citant la documentation QNX mot pour mot.
758. Préparer une carte de visite de projet, avec nom, tagline, catégorie et deux liens.
759. Condenser la démonstration en un tableau de trois colonnes : livré, prouvé, encore bloqué.
760. Rédiger un résumé de dix lignes pour un message privé, collable dans n'importe quel fil de discussion.
761. Graver dans la mémoire collective une image simple : la politique devient un artefact auditable.
762. Préparer la réponse de dix secondes à la question « qu'est-ce que vous faites de neuf ? ».
763. Écrire l'accroche du pitch en une phrase, sans jargon QNX ni sigle indéchiffrable.
764. Tester la compréhension en montrant la démo à quelqu'un d'un autre domaine, sans aucune explication.
765. Rédiger la réponse à « en quoi différez-vous de secpolcompile ? », avec la source de chaque phrase.
766. Préparer la réponse à « votre politique est-elle la vraie grammaire QNX ? », honnête sur le format provisoire.
767. Documenter la réponse à « que se passe-t-il si le SDP arrive après le hackathon ? ».
768. Expliquer par écrit la réponse à « comment est prouvée la non-interférence ? », via le parcours exhaustif.
769. Préparer la réponse à « qu'avez-vous inventé ? », en distinguant grammaire relevée et choix de conception.
770. Répondre par avance à « votre outil est-il un simple linter ? », avec la preuve et son certificat.
771. Anticiper la question « votre banc QNX 7.1 prouve-t-il QNX 8.0 ? » et sa réponse honnête.
772. Préparer la réponse à « que se passe-t-il si une règle change pendant l'analyse ? », sur l'immuabilité du modèle.
773. Écrire la réponse à « combien de commandes faut-il ? » : une seule, la même que dans le script.
774. Prévoir la réponse à « votre code est-il auditable ? », avec le lien vers le cœur C en C11 pur.
775. Anticiper la question sur les performances, avec un temps mesuré sur mille règles.
776. Constituer une fiche « objections courantes » de dix lignes, imprimable et mise à jour après chaque essai.
777. Tester dix titres de projet possibles sur un groupe de cinq personnes, puis retenir le plus clair.
778. Choisir une tagline de dix mots maximum, rigoureusement identique sur le site, le dépôt et Devpost.
779. Remplir la catégorie Developer Tooling & Host Tools, en justifiant ce choix par une phrase.
780. Réviser la section « Bonuses » pour viser le maximum de primes visées, avec une justification pour chacune.
781. Choisir les hashtags Devpost : QNX, sécurité, analyse statique, automobile, systèmes embarqués.
782. Préparer une image de couverture 1280×640, avec le chemin d'attaque en rouge sur fond sombre.
783. Rédiger la description Devpost en 800 mots maximum, avec la preuve d'usage QNX placée en tête.
784. Placer le lien de la vidéo dans les trente premiers mots de la description Devpost.
785. Rendre le lien du dépôt visible dès la première phrase, avec l'aperçu du README qui suit.
786. Écrire la section « ce que votre projet fait » avec quatre questions numérotées, comme sur le site.
787. Joindre au dossier Devpost la table des huit niveaux de vérification, générée automatiquement depuis le site.
788. Vérifier que chaque lien du dossier Devpost s'ouvre sans authentification ni création de compte.
789. Faire relire le dossier Devpost par un locuteur natif anglais, avant le gel définitif des liens.
790. Consigner dans le dépôt le numéro de soumission Devpost, dès l'envoi du formulaire.
791. Écrire un billet de blog technique : de la politique illisible à la preuve, en six chapitres.
792. Diffuser le billet de blog via un flux RSS du dépôt, pour être relayé sans réseau social.
793. Rédiger un billet court « cinq choses que la documentation QNX ne vous dit pas ».
794. Préparer un billet LinkedIn d'une page, avec la capture du chemin d'attaque en pièce jointe.
795. Préparer un fil Mastodon ou X, avec la vidéo jointe et le lien du dépôt dans le premier commentaire.
796. Réutiliser la capture de la preuve comme image sociale, aux formats 1200×630, 1200×1200 et 1080×1080.
797. Ajouter au README une capture GIF de six secondes, rejouée automatiquement, montrant la démonstration.
798. Placer en tête du README le trio promesse, démonstration et preuve, avec les trois liens cliquables.
799. Ajouter au README une section « voir la démo en 60 secondes », avec trois captures légendées.
800. Publier un fil de discussion de six messages, un par plan de la vidéo, pour les réseaux sociaux.

## pipeline, CI et sécurité de l'outillage

801. Limiter le débit du pont à soixante requêtes par minute, et renvoyer 429 au-delà.
802. Expirer automatiquement la session du pont après trente minutes d'inactivité, puis arrêter le service.
803. Révoquer le jeton à la demande par un signal, sans redémarrage du pont ni perte du banc.
804. Journaliser chaque requête du pont en JSON : méthode, chemin, statut, durée, identifiant de session.
805. Ne jamais journaliser le jeton, les politiques reçues, ni le corps complet des requêtes.
806. Taguer chaque refus par un motif normalisé (401, 403, 429), exploitable dans un tableau de bord.
807. Vérifier l'en-tête Host pour bloquer le DNS rebinding contre 127.0.0.1, sauf hôte local exact.
808. Ouvrir une seconde socket UNIX en mode 0600, pour éviter toute exposition du pont au réseau TCP.
809. Stocker le jeton du pont dans un fichier 0600, et ne l'afficher qu'une seule fois au démarrage.
810. Exécuter le pont sous un compte dédié, sans droit d'écriture hors du répertoire de travail.
811. Écrire les politiques dans un fichier temporaire privé, ouvert en O_EXCL, puis supprimé à la fin.
812. Vérifier l'empreinte SHA-256 du binaire core C avant chaque invocation faite par le pont.
813. Plafonner le pont à quatre analyses simultanées, et mettre les requêtes excédentaires en file.
814. Interrompre toute analyse dépassant le délai imparti, et tuer le processus enfant sans zombie.
815. Valider le corps JSON reçu avec un schéma strict, et refuser toute clé inattendue.
816. Refuser toute valeur de champ texte contenant un chemin absolu ou une séquence de traversée.
817. Refuser toute politique dépassant 512 Kio, et indiquer au client la taille maximale acceptée.
818. Exiger une confirmation explicite du client avant tout démarrage ou arrêt du banc QNX.
819. Verrouiller le banc avec un fichier d'exclusion, pour interdire deux démonstrations concurrentes.
820. Exiger un nonce unique par requête du pont, et rejeter toute requête rejouée.
821. Enregistrer l'empreinte SHA-256 de chaque politique analysée, dans un journal d'audit séparé.
822. Épingler la clé SSH du banc dans un fichier 0600, et vérifier l'hôte avant toute connexion.
823. Désactiver StrictHostKeyChecking et le forwarding d'agent sur la session SSH du banc.
824. Nettoyer au démarrage les fichiers temporaires et les processus enfants laissés par un pont précédent.
825. Vérifier qu'aucun processus enfant du pont ne survit à l'arrêt du serveur HTTP.
826. Stocker le jeton côté navigateur dans sessionStorage, jamais dans localStorage ni dans l'URL.
827. Ne jamais transmettre le jeton du pont dans une URL, un paramètre de requête ou un fragment.
828. Servir aussi le pont en HTTPS local avec un certificat éphémère, pour chiffrer le jeton en transit.
829. Exposer un endpoint /journal restreint au jeton, listant les vingt derniers appels sans leur contenu.
830. Refuser toute commande distante du banc absente de la liste blanche des préréglages figés.
831. Fixer un budget de durée par job dans un fichier ci-budget.yml, et échouer si un job le dépasse.
832. Programmer l'échec du pipeline dès qu'une étape dépasse le double de sa durée médiane.
833. Restreindre le GITHUB_TOKEN au strict nécessaire : lecture seule par défaut, écriture seulement pour pages.
834. Interdire les workflows pull_request_target, qui exposent les secrets aux pull requests externes.
835. Bloquer la CI si un secret est détecté dans l'historique complet du dépôt par gitleaks.
836. Exiger deux relecteurs distincts sur toute modification de reach.c, proof.c ou diff.c.
837. Exiger le statut ci réussi comme condition obligatoire de fusion sur la branche principale.
838. Protéger la branche principale par ruleset : interdiction du force-push et de la suppression.
839. Programmer la fusion automatique des correctifs Dependabot, seulement après passage de la CI.
840. Programmer l'installation de la toolchain depuis un fichier versionné, pour ne pas dépendre de l'image runner.
841. Désactiver le cache sur le job mutants, pour garantir une reconstruction à froid de chaque mutant.
842. Conserver les artefacts de job sept jours, et les purger automatiquement au-delà.
843. Mesurer la couverture du jumeau JavaScript comparée au cœur C, et publier l'écart résiduel.
844. Joindre au run un rapport détaillant chaque mutation et le test qui la tue.
845. Rejouer en priorité les mutants du diff de la pull request, avant la campagne complète du core.
846. Vérifier que le site régénéré est identique au site commité, et échouer sur toute dérive.
847. Valider la syntaxe de tous les workflows YAML avec actionlint, avant le lancement du premier job.
848. Tester les workflows localement avec act, sur un push de branche, avant d'ouvrir la pull request.
849. Reconstruire le site dans un répertoire temporaire, puis comparer l'artefact publié au dépôt.
850. Exécuter le déploiement pages seulement si le job de tests a réussi sur le même commit.
851. Publier un manifeste SHA256SUMS de tous les artefacts, et le vérifier avant tout téléchargement.
852. Épingler les images de runner et les conteneurs par empreinte, jamais par étiquette flottante.
853. Plafonner la taille des artefacts uploadés par le job de couverture, pour tenir le quota GitHub.
854. Reconstruire le cœur deux fois dans le même job, et exiger des binaires identiques octet pour octet.
855. Comparer le coût en minutes de la CI au budget hebdomadaire, et alerter dès qu'il est dépassé.
856. Publier une GitHub Release automatique au tag v*, contenant le binaire et les notes de version.
857. Refuser de publier si le tag ne correspond pas à la version déclarée dans CONTRACT.md.
858. Générer les notes de version automatiquement, à partir des commits conventionnels du dépôt.
859. Signer le tag de release avec GPG, et vérifier la signature avant toute publication.
860. Publier le binaire dans un paquet Debian et un paquet RPM minimal, construits depuis la même source.
861. Distribuer le pont dans une image OCI non-root publiée sur GHCR, avec le binaire C embarqué.
862. Épingler le pont et le site sur une version commune, et tester leur compatibilité dans la CI.
863. Épingler la version du pont dans le schéma du protocole, et refuser un client incompatible.
864. Joindre au tag de release un fichier SHA256SUMS, vérifié automatiquement par un job dédié.
865. Publier le binaire dans une archive versionnée contenant la licence, le NOTICE et le contrat.
866. Épingler la version de QEMU utilisée par le banc dans un fichier de verrou versionné du dépôt.
867. Décrire l'image QNX comme non redistribuable dans l'artefact de release, sans jamais l'inclure.
868. Joindre au tag de release le rapport d'acceptation du banc, avec la date de mesure.
869. Auditer la liste des actions GitHub utilisées, et justifier chacune dans un fichier de revue.
870. Graver la version de build dans la sortie de la CLI, pour tracer précisément l'artefact exécuté.
871. Rendre chaque nom d'artefact auto-descriptif : version, commit et empreinte courte.
872. Consigner dans chaque artefact le numéro de version, le commit source et la date de build.
873. Rendre le manifeste des artefacts append-only, et refuser toute retouche d'une entrée publiée.
874. Vérifier que le binaire publié ne contient aucune chaîne de chemin absolu de build.
875. Vérifier que le dépôt n'importe aucun module Python hors bibliothèque standard.
876. Construire le binaire de release dans un conteneur épinglé, et publier l'empreinte de son image.
877. Interdire toute modification du schéma JSON sans incrémenter le numéro de version du contrat.
878. Comparer l'empreinte du binaire publié à celle du job de test du même commit.
879. Publier la commande unique de reconstruction, et la faire vérifier par un job dédié.
880. Reconstruire le binaire depuis la release dans un conteneur vierge, et exiger la même empreinte.
881. Sauvegarder un instantané du disque QNX avant chaque session, puis le restaurer systématiquement ensuite.
882. Rejouer hors ligne les sessions SSH enregistrées, pour valider l'harnais sans démarrer la VM.
883. Consigner dans un journal d'audit horodaté chaque commande exécutée sur l'invité QNX.
884. Arrêter automatiquement la VM après quinze minutes d'inactivité, via un minuteur systemd utilisateur.
885. Surveiller la santé du banc par un watchdog, et redémarrer QEMU si l'invité ne répond plus.
886. Surveiller l'espace disque du préfixe QEMU, et purger les instantanés dépassant cinq jours.
887. Monter l'image QNX en lecture seule, et n'écrire que dans un overlay jetable.
888. Isoler le réseau de l'invité : pas de passerelle, pas de DNS sortant, seulement SSH local.
889. Faire expirer les sessions du banc après une heure, avec déconnexion puis arrêt de l'invité.
890. Interdire toute commande du pont déclenchant l'arrêt de la VM sans confirmation explicite du client.
891. Programmer une alerte de maintenance quand l'image QNX du banc dépasse six mois d'ancienneté.
892. Vérifier l'intégrité de l'image QNX par empreinte avant chaque démarrage, et refuser tout écart.
893. Vérifier en CI que chaque fichier du dépôt porte un en-tête SPDX, et l'exiger pour les nouveaux.
894. Maintenir un fichier NOTICE listant les licences tierces et leurs obligations de redistribution.
895. Bloquer la CI si un fichier de plus de dix mégaoctets apparaît, pour interdire l'image QNX.
896. Interdire la publication de contenu QNX non redistribuable dans le site comme dans les artefacts.
897. Publier une SECURITY.md : procédure de signalement, délai de réponse et versions affectées.
898. Ajouter au dossier de soumission une clause de licence QNX, sans citer le contenu du SDP.
899. Exiger la signature des commits pour toute contribution externe, contrôlée par la CI.
900. Nommer un responsable par fichier d'outillage, et le consigner dans le fichier CODEOWNERS.

## process, pilotage et finalisation

901. Envoyer la demande de licence QNX SDP 8.0 aujourd'hui, puis consigner l'accusé de réception daté.
902. Fixer au 20 novembre la date-butoir d'obtention du SDP, sinon basculer officiellement en plan B.
903. Rédiger le plan B sans SDP : grammaires sourcées, banc QNX 7.1 attesté, oracle déclaré optionnel.
904. Faire valider par un pair la formulation honnête du dossier Devpost sur la licence absente.
905. Recenser dans un tableau chaque affirmation Devpost sur QNX 8.0, avec sa source et son niveau de confiance.
906. Préparer un correctif de phrase prêt à coller si la licence arrive après le gel du dossier.
907. Écrire et chronométrer la procédure d'installation du SDP, testée sur une machine vierge.
908. Vérifier que le harnais oracle échoue proprement, code 3, sans faux positif ni trace d'erreur.
909. Établir un rétroplanning daté du 12 novembre au 15 décembre, avec un jalon vérifiable par semaine.
910. Attribuer à chacune des treize exigences TAM restantes un responsable et une date d'achèvement.
911. Interdire tout ajout de fonctionnalité après le 25 novembre, sauf vote explicite de l'équipe.
912. Tenir un point de pilotage hebdomadaire de quinze minutes, avec compte rendu écrit versionné.
913. Suivre dans un tableau unique l'écart entre avancement réel et rétroplanning, mis à jour chaque lundi.
914. Répartir les dix-huit jours post-gel entre documentation, relecture, vidéo et soumission, sans code.
915. Prévoir deux journées tampon entre le gel fonctionnel et la soumission pour les correctifs bloquants.
916. Écrire la checklist « prêt à soumettre » : vingt points binaires, testés la veille de la soumission.
917. Décider le 10 décembre si la vidéo est exigée par le jury ou si la démonstration live suffit.
918. Fixer l'heure limite de soumission à 23 heures le 12 décembre, puis relire le dossier le 13.
919. Soumettre une version anticipée le 11 décembre, pour valider le formulaire Devpost sans stress.
920. Écrire le repli minimal acceptable si un blocage majeur survient dans la dernière semaine.
921. Écrire la liste exacte des fonctionnalités livrées au gel du 5 décembre, dans le dépôt.
922. Décrire en une phrase chaque commande CLI conservée, et refuser tout ajout après le gel.
923. Faire du format JSON figé le contrat de démonstration opposable, cité dans le dossier Devpost.
924. Décider quels onglets du site restent actifs au 5 décembre, et retirer ou masquer tous les autres.
925. Classer chaque idée restante en « fait », « à faire », « renoncé », dans un tableau unique.
926. Interdire toute nouvelle dépendance externe après le gel du 5 décembre, sans exception documentée.
927. Figer le jeu de fixtures définitivement, sans ajout ni renommage après le 5 décembre.
928. Garder le format provisoire .mpf interne, et ne jamais le présenter comme du vrai langage secpol.
929. Exiger zéro échec de make test sur la branche de soumission, obtenu trois fois de suite.
930. Exiger que le différentiel C, Python et JavaScript affiche zéro divergence sur les trois graines.
931. Vérifier que le banc QNX distant répond et rejoue l'attaque complète de bout en bout.
932. Contrôler que chaque chiffre du dossier Devpost correspond au dernier make test exécuté.
933. Tester le site de démonstration depuis un téléphone en réseau mobile, sans VPN ni extension.
934. Vérifier que le dépôt public s'ouvre et se comprend en trente secondes, sans compte ni installation.
935. Vérifier que la vidéo ne montre ni image QNX ni binaire QNX non redistribuable.
936. Faire relire le dossier Devpost par un pair qui n'a jamais participé au projet.
937. Faire relire la documentation française par un locuteur natif, sans contexte technique.
938. Faire relire les six moteurs par deux pairs distincts, avec une liste de points bloquants.
939. Éliminer les liens morts du README, du site et de la documentation via un script automatique.
940. Relire chaque affirmation du dossier Devpost une par une, en exigeant une preuve juste à côté.
941. Tenir un registre des risques : licence refusée, vidéo non tournée, bogue tardif, jury sceptique.
942. Attribuer à chaque risque un propriétaire, une probabilité et une date d'alerte explicite.
943. Déclencher une alerte dès que la licence dépasse quinze jours sans réponse exploitable.
944. Préparer un second script vidéo, entièrement jouable dans le navigateur, sans terminal ni installation.
945. Tourner la vidéo avant le 25 novembre, pour conserver deux semaines de correction.
946. Rédiger à l'avance les réponses aux cinq questions les plus probables du jury.
947. Répéter une soutenance de cinq minutes devant un pair, chronométrée avec un minuteur.
948. Constituer un kit de secours hors ligne : captures, démo, chiffres, sur clé USB.
949. Tester ce kit sur une machine sans réseau, avec la capture d'écran de l'essai à l'appui.
950. Prévoir le repli si GitHub Pages tombe le jour de la soumission, vidéo locale en attente.
951. Écrire la liste des dix bugs les plus probables, puis les tester un par un avant le 30 novembre.
952. Figer le binaire de soumission : archive datée avec somme SHA-256 publiée dans le dépôt.
953. Publier les versions exactes des outils requis pour reproduire tous les chiffres annoncés.
954. Vérifier que make test passe sur une machine vierge, sans cache ni dépendance partagée.
955. Activer un test quotidien automatique qui surveille la régression et alerte le canal d'équipe.
956. Revalider toute la chaîne de démonstration si le cœur C change après le 8 décembre.
957. Interdire toute modification du cœur C après le 8 décembre sans validation écrite de deux pairs.
958. Organiser deux revues de code de deux heures, planifiées les 20 et 27 novembre.
959. Utiliser un formulaire de revue commun, avec une case explicite « bloque la soumission ».
960. Fermer chaque point ouvert par une pull request tracée, ou par une justification écrite.
961. Faire relire la grammaire secpol et ses marqueurs À VÉRIFIER par un pair indépendant.
962. Faire relire CONTRACT.md pour vérifier qu'aucune divergence ne s'est glissée depuis le gel.
963. Relire le README pour supprimer toute promesse d'une fonctionnalité non livrée.
964. Organiser une revue de démonstration par un pair non développeur, chronométrée et filmée.
965. Émettre un avis formel go ou no-go, signé par deux pairs, le 8 décembre au plus tard.
966. Classer les tâches restantes par valeur pour les jurés divisée par l'effort restant.
967. Limiter le travail en cours à deux tâches simultanées, et ne jamais en ouvrir une troisième.
968. Écarter toute tâche absente du dossier Devpost et du codelab, même si elle est intéressante.
969. Réserver le dernier tiers du temps disponible à la relecture, jamais aux fonctionnalités nouvelles.
970. Parfaire un seul effet « waouh » de démonstration plutôt que d'en ajouter trois approximatifs.
971. Publier un communiqué d'avancement hebdomadaire daté, sur le dépôt et sur les réseaux.
972. Publier dans le dépôt une feuille de route post-hackathon, sans engagement commercial.
973. Préparer un billet LinkedIn et un billet X prêts à publier le 15 décembre.
974. Documenter dans le dépôt la contribution de chaque membre, pour le rapport final du jury.
975. Répondre aux questions des lecteurs du dépôt dans les quarante-huit heures, en français et en anglais.
976. Figer la version anglaise du dossier Devpost, puis traiter la version française séparément.
977. Vérifier que chaque affirmation du dossier renvoie à un fichier ou à un test reproductible.
978. Ajouter au dossier les trois liens attendus : vidéo, site de démonstration et dépôt public.
979. Préparer des captures d'écran numérotées, insérables en fin de liste du dossier.
980. Rédiger une synthèse d'une page pour un lecteur pressé, sans jargon ni sigle.
981. Réserver la matinée du 13 décembre au remplissage Devpost, sans toucher au code.
982. Vider la liste des tâches au 14 décembre, puis consigner dans le dépôt le résultat final.
983. Archiver la version soumise dans une branche gelée, avec un tag Git daté.
984. Préparer le texte de remerciements aux jurés et aux contributeurs, relu avant envoi.
985. Rédiger le rapport post-hackathon pendant la soumission, puis le compléter en janvier.
986. Prévoir une revue de code du diff soumis dans les dix jours suivant la clôture.
987. Vérifier que le nom du projet, la tagline et la catégorie sont identiques sur le site et le dossier.
988. Uniformiser les métriques annoncées partout : vérifications, mutants tués, entrées de fuzz.
989. Faire relire la démonstration du site par cinq personnes hors projet, avec retour écrit.
990. Mesurer le temps de la démo navigateur et le ramener sous quatre minutes.
991. Vérifier que le codelab tient en vingt minutes réelles, chronomètre en main.
992. Prévoir une capture d'écran du terminal pour chaque commande du codelab, dans le dossier.
993. Ajouter une section « limites » au dossier Devpost, volontairement honnête et chiffrée.
994. Documenter en une phrase pourquoi le cœur C ne dépend d'aucune horloge ni d'aucun fichier.
995. Définir la règle « un commit, une intention » et l'appliquer dès la branche de soumission.
996. Écrire le changelog de la version soumissionnée, avec les ruptures de format connues.
997. Vérifier l'absence de secrets, de jetons et de chemins absolus avant chaque publication.
998. Prévenir le mentor QNX par un message court, avec le lien du dépôt final.
999. Programmer un point de suivi à J+7 après la clôture, pour répondre aux questions des jurés.
1000. Écrire le plan de la semaine suivant la soumission : correctifs, oracle, et reprise du travail gelé.
