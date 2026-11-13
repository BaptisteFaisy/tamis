<!-- SPDX-License-Identifier: MIT -->
# Tamis — décisions de conception (ADR)

Format court : une décision = contexte → décision → conséquence. Les plus récentes en haut.

---

## D-007 — Les preuves sont rejouées dans la vraie cible, via une liste fermée (2026-11-13)

**Contexte.** Le site est statique (GitHub Pages) : il ne peut ni exécuter le binaire C, ni
faire tourner QEMU, ni joindre `127.0.0.1` du serveur. Le pont local (`tools/tamis-bridge.py`)
comble l'écart, mais il demande au juge d'installer et lancer quelque chose sur sa machine —
alors qu'une **vraie cible QNX 8.0** tourne déjà sur `r630`, avec le cœur C recompilé par
`qcc`. En outre, l'invité coûte ~1,5 Go de RAM : il ne peut pas tourner en permanence.

**Décision.** Un service HTTP dédié (`banc/service.py`) rejoue une **liste fermée** d'étapes
dans l'invité QNX 8.0 et renvoie la sortie, comparée octet pour octet à celle du poste.
- **Liste fermée** : le client ne demande qu'un identifiant d'étape (`banc/commandes-emulateur.json`),
  jamais une commande libre. Aucune surface d'exécution arbitraire n'est exposée.
- **Exposition** : `127.0.0.1` uniquement côté serveur ; le public passe par `tailscale funnel`
  (HTTPS), avec CORS restreint à l'origine du site.
- **Cycle de vie** : la vitrine reste le **seul propriétaire** de la politique d'inactivité
  (arrêt après 5 min sans visite) ; le service lui **demande** un réveil et attend la fin du
  démarrage avant de taper quoi que ce soit dans la console (`banc/invite_cycle.py`).
- La sortie passe par la **console série** (octets exacts, accents intacts), pas par le tampon
  VGA (charset VGA) — et le CRLF de la console est normalisé, seule transformation faite.

**Conséquence.** Le juge rejoue une preuve depuis le site public : elle est produite par le
vrai binaire QNX 8.0 dans une vraie cible, et le badge « identique (octet pour octet) » est
un fait mesuré, pas une promesse. En contrepartie, l'invité peut redémarrer pendant une
preuve : le service le réveille et attend (jusqu'à ~75 s), et le dit au client.

---

## D-006 — Parseur `secpol` écrit sur le MÊME modèle que le format provisoire (2026-11-13)

**Contexte.** La grammaire est gelée (`docs/GRAMMAR.md`, SDP 8.0 installé) et le compilateur
officiel `secpolcompile` accepte `fixtures/politique-qnx8.secpol`. Il fallait brancher un
vrai parseur sans réécrire les moteurs (reach, proof, lint, minimize, diff, explain), ni
casser les fixtures `.mpf` déjà utilisées par la démo.

**Décision.** Le parseur du langage réel produit **exactement le même `tamis_model`** que le
parseur du format provisoire : `type_names` dans l'ordre source, `rules[].src/dst` en
**indices** dans ce tableau, `has_default_spawn`. Conséquences directes :
- les six moteurs n'ont pas changé d'une ligne ;
- le CLI choisit le parseur sur l'extension (`.secpol`) et accepte, à titre de repli, les
  `.mpf` de démonstration ;
- le jumeau navigateur (`site/tamis-secpol*.js`) est découpé comme le C
  (`abil`/`decl`/`lex`/`prod`) et rend le même modèle, refus compris.

Le sous-ensemble reste **strict** : seule la grammaire attestée est lue, tout le reste est
refusé avec ligne, colonne et cause.

**Conséquence.** Le différentiel C↔JS `tests/test_js.sh` couvre désormais les 42 politiques
du corpus secpol *et* la politique QNX 8.0, en plus des scénarios du format provisoire. Le
rythme du projet ne dépend plus d'un parseur « à venir » : il dépend de la non-régression.

---

## D-005 — Différentiel C↔Python par sérialisation base64 (2026-11-11)

**Contexte.** Le jumeau Python (`host/tamis_report.py`) et le cœur C (`core/src/rapport.c`)
doivent produire des sorties **octet pour octet** identiques (TAM-09). Il faut donc leur
soumettre exactement les mêmes verdicts, y compris des chaînes piégeuses (guillemets,
backslash, contrôles, UTF-8 multioctet).

**Décision.** `tests/gen_verdicts.py` génère des verdicts déterministes par graine et les
émet dans deux modes partageant la même génération : `serialize` (format TAB + base64 lu par
`tests/diff_driver.c`) et `render` (report_json Python). Le base64 évite toute ambiguïté de
délimiteur ; la sentinelle `@` distingue `None` (champ absent) de la chaîne vide `""`.
`tests/test_differentiel.sh` compile le pont C, compare octet par octet et localise le
premier désaccord.

**Conséquence.** 0 divergence vérifiée sur 3 graines (250/300/500 verdicts). Ajouter un
moteur au différentiel = étendre `gen_verdicts.py`, pas réécrire un harnais.

## D-004 — Mode sous-ensemble strict (2026-11-11)

**Contexte.** La grammaire exacte du langage `secpol` n'est pas encore gelée
(`docs/GRAMMAR.md` est relevé depuis l'inventaire documenté, doc officielle inaccessible).
Risque : inventer une syntaxe et produire une analyse faussement rassurante.

**Décision.** Tamis applique le **mode sous-ensemble strict** : il refuse d'analyser ce qu'il
ne sait pas parser et le dit (`TAMIS_PARSE_ERROR` avec position et cause). **Jamais**
d'analyse approximative d'une politique partiellement comprise (CONTRACT §1).

**Conséquence.** Tant que `docs/GRAMMAR.md` n'était pas gelé, le parseur n'était pas écrit.
C'est fait depuis (voir D-006) : le parseur existe et applique toujours la même règle —
un refus explicite, jamais une lecture au rabais. Le critère n'a pas changé, seule la
condition de départ (grammaire gelée) est remplie.

## D-003 — Socle C↔Python indépendant de la grammaire (2026-11-11)

**Contexte.** Tout le projet était à l'arrêt (0 ligne) sur le lot L0 (grammaire), bloquant.

**Décision.** Découpler : livrer d'abord le **socle verdicts + rapports** (13 codes, 3 rendus
texte/JSON/Markdown d'audit, jumeau Python, différentiel), qui ne dépend d'aucune syntaxe.
La grammaire et le parseur viennent ensuite, par-dessus.

**Conséquence.** 623 vérifications vertes (287 C + 86 Python + 250 différentiel) dès le
premier jalon, sans SDP ni cible. Le `CONTRACT.md` est gelé et sert de référence aux deux
implémentations.

## D-002 — Cœur pur, métadonnées fournies par l'appelant (2026-11-11)

**Contexte.** Le rapport d'audit a besoin d'une empreinte SHA-256, d'un horodatage et d'une
version — autant d'éléments qui tentent vers l'E/S et l'horloge, interdits dans `core/`.

**Décision.** `core/` est **pur** : ni horloge, ni fichier, ni socket, ni état global mutable,
aucune allocation (l'appelant fournit les tampons). Les métadonnées passent par la structure
`tamis_report_meta` remplie par l'appelant (`host/`).

**Conséquence.** Le cœur est testable unitairement et embarquable (plus tard en WASM). Le
calcul SHA-256 et l'horodatage vivent dans `host/`, jamais dans `core/`.

## D-001 — Dépôt standalone `Tradoooo/tamis` (2026-11-11)

**Contexte.** Le socle partagé vit dans `qnx-kit/tamis/` (lot F, sans remote). La soumission
au QNX Global Hackathon exige un **dépôt public** dédié (cahier §13).

**Décision.** Dépôt public standalone `Tradoooo/tamis` (MIT), suivant l'architecture du
cahier §4. `GaiaSG94` invité **contributor** (permission write). Le lot F de `qnx-kit` a
vocation à être replié dans ce dépôt.

**Conséquence.** CI GitHub dédiée (`ci.yml` ≤ 5 min/job, `oracle.yml` hors PR sur
`workflow_dispatch`). Aucun chemin absolu codé en dur (garde CI).

---

## Blocages ouverts (actions humaines)

| Blocage | Qui | Effort |
|---|---|---|
| **Vidéo 3 min** (tournage + montage + sous-titres) | **Baptiste** — caméra | 1 j + montage |
| **Soumission Devpost** (compte d'équipe + 13 champs) | **Baptiste** | 1 h, avant le 12/12 |

Les deux blocages techniques d'hier sont **levés** : le SDP QNX 8.0 est installé (donc la
grammaire est gelée et le parseur `secpol` écrit), et `oracle/run_oracle.sh` compare
l'acceptation de Tamis à celle de `secpolcompile` sur les 43 politiques (42 du corpus
+ la politique QNX 8.0, accord total). Ce qui reste relève de l'action humaine : la vidéo et le dépôt.
