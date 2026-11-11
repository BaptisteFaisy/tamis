<!-- SPDX-License-Identifier: MIT -->
# Tamis — gabarit de rapport d'audit (TAM-16)

> Ce gabarit est la sortie Markdown de certification produite par `tamis_report_markdown`
> (CONTRACT.md §5.3). Il est destiné à être versé tel quel dans un dossier de sécurité
> (ISO 26262, IEC 62304, EN 50128, UNECE R155). Chaque propriété vérifiée occupe une
> section ; chaque section est auto-porteuse : un auditeur doit pouvoir la lire sans le reste.

---

# Rapport d'audit Tamis

- Politique : `<nom du fichier .secpol analysé>`
- Empreinte SHA-256 : `<64 hex>` *(scelle la version exacte de la politique analysée)*
- Horodatage (UTC) : `<AAAA-MM-JJThh:mm:ssZ>`
- Version de Tamis : `<x.y.z>`
- Périmètre de complétude : `<ce que cette analyse couvre>`
- Angles morts bornés : `<ce qu'elle ne couvre PAS, et pourquoi — TAM-21>`

## Synthèse

| Sévérité | Nombre |
|---|---|
| error | N |
| warning | N |
| info | N |

Verdict global : `<OK | FAILED n error(s)>`

## Propriétés vérifiées

### Propriété P1 — Non-interférence `<type_source>` ⇸ `<type_cible>`

- **Énoncé** : aucune suite d'opérations autorisées par la politique ne permet à
  `<type_source>` d'atteindre `<type_cible>`.
- **Méthode** : parcours exhaustif du graphe de privilèges (transitivité de `reach`),
  avec compteur de visites publié — l'absence de chemin est **prouvée**, pas seulement
  non trouvée (TAM-05).
- **Corpus** : `<politique analysée, empreinte ci-dessus>`.
- **Résultat** : `<AUCUN CHEMIN — certificat vert | CHEMIN TROUVÉ>`.
- **Contre-exemple** *(si chemin trouvé)* :
  `<type_source> → <intermédiaire 1> → <intermédiaire 2> → <type_cible>`
  via les règles `<R112, R87, R9>`, ligne `<214>` du fichier.
- **Correctif minimal proposé** : `<la règle à retirer pour tuer le chemin>`.

### Propriété P2 — Minimalité

- **Énoncé** : la politique ne contient aucune règle morte, redondante ou dominée.
- **Méthode** : suppression des règles non atteignables + preuve d'équivalence sur un
  corpus de 10 000 requêtes de décision rejouées avant/après (TAM-07).
- **Résultat** : `<N règles → M règles, aucune divergence sur 10 000 décisions>`.

### Propriété P3 — Typage complet

- **Énoncé** : tout processus est typé et `default_spawn_type` est cohérent.
- **Méthode** : lint des processus non typés et de l'absence de `default_spawn_type`
  (trous documentés S7, S10 ; TAM-06).
- **Résultat** : `<liste des processus non typés, ou « aucun »>`.

### Propriété P4 — Non-régression de privilège

- **Énoncé** : la modification de la politique n'élargit aucun privilège.
- **Méthode** : diff sémantique entre la version de référence et la version candidate
  (TAM-08).
- **Résultat** : `<aucun élargissement | élargissement signalé, règle responsable>`.

## Angles morts déclarés (TAM-21)

Conformément à la documentation QNX (trou S4), certains échecs sont **invisibles** à toute
analyse de politique : permissions de fichiers, transitions root→non-root. Tamis les
**signale comme non couverts** au lieu de les ignorer :

- `<angle mort 1>` : non couvert, car `<raison>`.
- `<angle mort 2>` : non couvert, car `<raison>`.

## Traçabilité

| Exigence | Propriété | Résultat |
|---|---|---|
| TAM-04/05 | P1 | `<…>` |
| TAM-07 | P2 | `<…>` |
| TAM-06 | P3 | `<…>` |
| TAM-08 | P4 | `<…>` |

---

*Généré par Tamis. Ce rapport est reproductible : la même politique (même empreinte)
et la même version de Tamis produisent le même rapport, à l'horodatage près.*
