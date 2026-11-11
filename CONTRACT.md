# Tamis — CONTRAT (gelé)

Version 0.1 — 11 novembre 2026. Ce fichier est la **source de vérité** du socle.
Toute divergence se règle en modifiant ce fichier, jamais en dupliquant la logique
ailleurs. Le cahier des charges (`TAMIS-cahier-des-charges.md` du dépôt `qnx-dossier`)
donne le contexte ; ce contrat précise les formats de façon **octet pour octet**
afin que le cœur C et le jumeau Python soient comparables par différentiel.

---

## 1. Chaîne de traitement (figée)

```
texte politique → lex → ast → model(graphe) → {reach, proof, minimize, diff, lint} → verdicts → rapport
```

Aucune étape ne remonte : un moteur ne relit jamais le texte. Toute erreur d'analyse
arrête la chaîne et produit un verdict `TAMIS_PARSE_ERROR` avec position (ligne, colonne)
et cause — **jamais** de reprise approximative sur une politique partiellement comprise.

## 2. Architecture et portabilité

- `core/` : C11 **pur** — ni horloge, ni fichier, ni socket, ni état global mutable.
  L'appelant fournit le texte et les tampons ; le cœur n'alloue que ce que l'appelant
  lui passe. Compilé en `-std=c11 -O2 -Wall -Wextra -Werror`.
- `host/` : Python 3 **standard uniquement** (aucune dépendance à installer). C'est un
  **jumeau de vérification indépendant**, pas une copie du C : même contrat de sortie,
  implémentation séparée. Le désaccord C↔Python est un bug, réduit dichotomiquement.
- `oracle/` : harnais de test différentiel contre `secpolcompile` (utilitaire hôte).
  Optionnel sans SDP ; ne bloque aucune autre exigence.

## 3. Verdicts (§5.2 du cahier)

13 codes, dans cet ordre (enum `tamis_verdict_code`, sentinelle `TAMIS_VERDICT__COUNT`) :

| # | Code | Sévérité par défaut | Porteur |
|---|---|---|---|
| 0 | `TAMIS_OK` | info | — |
| 1 | `TAMIS_PARSE_ERROR` | error | file, line, column, message |
| 2 | `TAMIS_UNTYPED_PROCESS` | warning | file, line, message |
| 3 | `TAMIS_MISSING_DEFAULT_SPAWN` | warning | file, message |
| 4 | `TAMIS_DEAD_RULE` | info | file, line, rules, message |
| 5 | `TAMIS_REDUNDANT_RULE` | info | file, line, rules, message |
| 6 | `TAMIS_OVERBROAD_ABILITY` | warning | file, line, rules, message |
| 7 | `TAMIS_RANGE_INVALID` | error | file, line, message |
| 8 | `TAMIS_PATH_FOUND` | error | src, dst, hops, path, rules, file, line |
| 9 | `TAMIS_INTERFERENCE_POSSIBLE` | error | src, dst, hops, path, rules, file, line |
| 10 | `TAMIS_PRIVILEGE_WIDENED` | error | src, dst, rules, file, line, message |
| 11 | `TAMIS_PRIVILEGE_NARROWED` | info | src, dst, rules, file, line, message |
| 12 | `TAMIS_INCONSISTENT` | error | file, line, message |

Un verdict porte toujours : `code`, `sev`, `message`. Les champs `file`, `line`,
`column`, `src`, `dst`, `path`, `rules` sont **optionnels** (absents si non applicables).
`line`/`column` sont 1-based ; 0 = inconnue. `hops` = `path_len - 1` quand `path` est présent.

## 4. API C (en-têtes sous `core/include/tamis/`)

`verdict.h` :
```c
const char *tamis_verdict_name(tamis_verdict_code code);   /* hors plage -> "TAMIS_UNKNOWN" */
const char *tamis_severity_name(tamis_severity sev);        /* "info"|"warning"|"error" */
tamis_severity tamis_verdict_default_severity(tamis_verdict_code code);
int tamis_verdict_code_valid(tamis_verdict_code code);      /* 1 si dans [0,COUNT) */
```

`report.h` :
```c
long tamis_report_text    (const tamis_verdict *v, size_t n, char *buf, size_t cap);
long tamis_report_json    (const tamis_verdict *v, size_t n, char *buf, size_t cap);
long tamis_report_markdown(const tamis_verdict *v, size_t n,
                           const tamis_report_meta *meta, char *buf, size_t cap);
int  tamis_report_exit_code(const tamis_verdict *v, size_t n);
```
Sémantique de retour : nombre d'octets qui auraient été écrits tampon plein
(comme `snprintf`) ; `< 0` si argument invalide. Contenu toujours NUL-terminé si `cap > 0`.
`tamis_report_meta` (fourni par l'appelant, le cœur ne lit ni horloge ni fichier) :
`policy_file`, `policy_sha256` (64 hex), `timestamp_utc` (ISO-8601), `tamis_version`.
Tout champ NULL est rendu `inconnu`.

## 5. Formats de sortie

### 5.1 Texte (`tamis_report_text`)
Une ligne par verdict, préfixe de sévérité, puis code, position et message :
```
ERROR  TAMIS_PATH_FOUND  policy.secpol:214  app_tiers -> pompe_critique (3 sauts) : R112,R87,R9
WARN   TAMIS_UNTYPED_PROCESS  policy.secpol:88  processus non typé : /bin/foo
INFO   TAMIS_DEAD_RULE  policy.secpol:120  règle jamais atteinte : R42
```
Format exact d'une ligne de verdict (les séparateurs sont **2 espaces**) :
`<PRÉFIXE><CODE>  <POSITION>  <DÉTAIL>`
- `<PRÉFIXE>` = label de sévérité (`ERROR`/`WARN`/`INFO`) aligné à gauche sur
  **5 caractères** puis **2 espaces** (champ de 7 caractères) : `ERROR  `, `WARN   `, `INFO   `.
- `<CODE>` = nom canonique du verdict (ex. `TAMIS_PATH_FOUND`).
- `<POSITION>` = `<file>` si présent sinon `?`, puis `:<line>` si line>0, puis
  `:<column>` si column>0. Ex. `policy.secpol:214`.
- `<DÉTAIL>` : si un chemin est présent (`path_len>0`) →
  `<src> -> <dst> (<hops> sauts)` suivi de ` : <règles join ",">` si rules présent ;
  sinon → le `message`.
Termine par une ligne de synthèse : `OK` si aucun verdict de sévérité error,
sinon `FAILED <n> error(s)`. Les lignes sont jointes par `\n`, avec un `\n` final.

### 5.2 JSON (`tamis_report_json`) — **une ligne par verdict**, compact
Ordre des clés **figé** : `code`, `sev`, `src`, `dst`, `hops`, `path`, `rules`,
`file`, `line`, `column`, `message`. Les clés optionnelles sont **omises** quand le
champ est absent (NULL / 0 / tableau vide). `message` est toujours présent.
Séparateurs compacts : `,` et `:` **sans espace**. Équivalent Python :
`json.dumps(obj, separators=(',', ':'), ensure_ascii=False)`.

Échappement des chaînes (identique C et Python) : `"` → `\"`, `\` → `\\`,
`\b \f \n \r \t` → escape court, tout autre caractère de contrôle `< 0x20` →
`\u00XX` (XX hexadécimal **minuscule**). Les octets UTF-8 ≥ 0x20 passent tels quels
(pas de `\uXXXX` pour le non-ASCII).

Exemple canonique (verdict de chemin) :
```json
{"code":"TAMIS_PATH_FOUND","sev":"error","src":"app_tiers","dst":"pompe_critique","hops":3,"path":["app_tiers","journal","diag","pompe_critique"],"rules":["R112","R87","R9"],"file":"policy.secpol","line":214,"message":"chemin de privilège atteignable"}
```

### 5.3 Markdown d'audit (`tamis_report_markdown`) — §5.4 du cahier
Structure figée :
```
# Rapport d'audit Tamis

- Politique : <policy_file>
- Empreinte SHA-256 : <policy_sha256|inconnu>
- Horodatage (UTC) : <timestamp_utc|inconnu>
- Version de Tamis : <tamis_version|inconnu>

## Synthèse

| Sévérité | Nombre |
|---|---|
| error | N |
| warning | N |
| info | N |

## Verdicts

### <CODE> — <sev> (×N)
- Position : <file>:<line>[:<column>]
- Message : <message>
- Chemin : <src> → <dst> (<hops> sauts) : <path join " → ">   [si applicable]
- Règles : <rules join ", ">                                     [si applicable]

## Propriétés vérifiées

(une section par propriété : énoncé, méthode, corpus, résultat, contre-exemple)
```

## 6. Jumeau Python (`host/tamis_report.py`)

Mêmes verdicts, mêmes rendus **octet pour octet** sur JSON et texte. API :
```python
class Verdict: ...            # champs identiques à tamis_verdict
class ReportMeta: ...         # champs identiques à tamis_report_meta
def report_text(verdicts: list[Verdict]) -> str
def report_json(verdicts: list[Verdict]) -> str       # une ligne par verdict, join "\n"
def report_markdown(verdicts, meta: ReportMeta) -> str
def exit_code(verdicts: list[Verdict]) -> int
```
Le test différentiel (`tests/test_differentiel.sh`) génère ≥ 200 verdicts aléatoires
(déterministes par graine), les rend en C et en Python, et compare octet par octet.
Premier désaccord réduit dichotomiquement jusqu'au verdict fautif.

## 7. Conventions de test

- Un binaire de test par module : `core/tests/test_<module>.c`, autonome (son propre
  `main`), retourne **0 si vert**, affiche une ligne par vérification et termine par
  `OK <n> checks`. En cas d'échec : `FAILED <cause>` et retour non nul.
- Cible globale : **≥ 800 vérifications** (cahier §7.1). Le socle (verdict + rapport)
  vise ≥ 300 à lui seul.
- Tests Python : `host/test_*.py`, exécutables, même convention `OK <n> checks`.
- `make test` construit et lance tout ; `make sanitize` relance sous ASan/UBSan.

## 8. Gardes de taille (règle user 2026-10-24)

- **< 500 lignes par fichier**, **< 50 lignes par fonction**. Découper plutôt que
  laisser grossir. Garde : `tests/garde.py` (lancée par `make garde` et la CI).
- Zéro duplication : une seule implémentation par logique. Le jumeau Python est une
  implémentation **indépendante assumée et documentée**, pas une copie du C.

## 9. Statut

**Gelé** : chaîne de traitement (§1), architecture (§2), 13 codes de verdict et leur
sévérité par défaut (§3), API C (§4), formats texte/JSON/Markdown (§5), contrat du
jumeau Python (§6), conventions de test (§7), gardes de taille (§8).

**À VÉRIFIER** (attend `docs/GRAMMAR.md`, lot L0) : la grammaire exacte du langage
`secpol` — productions, terminaux, règles de portée. Le parseur (`core/src/lex.c`,
`ast.c`, `model.c`) **n'est pas** dans ce socle : il attend la grammaire gelée. Ne pas
l'anticiper, ne pas inventer de syntaxe.
