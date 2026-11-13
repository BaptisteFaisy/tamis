<!-- SPDX-License-Identifier: MIT -->
# Tamis — GRAMMAIRE du langage de politique de sécurité QNX 8.0 (`secpol`)

**Lot L0 — essai E1 : FAIT. Statut : GELÉ le 12/11/2026.**

> La documentation officielle en ligne renvoyait 404 pendant le développement ; c'est
> pourquoi cette grammaire a longtemps été « À VÉRIFIER ». Le blocage est levé : le **SDP
> QNX 8.0 est installé** (`~/qnx800` sur le serveur r630) et sa documentation officielle a
> été lue page à page (`tools/extract_grammar.py` → `docs/GRAMMAR-auto.md`, 76 productions),
> puis **croisée avec le compilateur officiel `secpolcompile`** qui accepte désormais une
> politique réelle (voir `preuve/qnx8/`).

## 0. La source de vérité

| Source | Rôle |
|---|---|
| `preuve/qnx8/` | capture réelle : `secpolcompile` accepte `politique-qnx8.secpol` (0 erreur), refuse une politique invalide (`undefined type 'app_tiers'`) |
| `docs/GRAMMAR-auto.md` | extraction brute des productions depuis les 96 pages HTML du SDP |
| `secpol_language.html` | la page de référence du langage |
| Compilateur `secpolcompile` | l'autorité : tout désaccord avec lui est un bug de Tamis |

## 1. Déclarations

```
type <nom>;                      # type de sécurité
range <nom>;                     # plage nommée (résolue au boot)
attribute <nom>;                 # groupe de types
class <nom> { <perm> … };        # permissions custom (objet non-canal)
default_spawn_type <t1> <t2>;    # type par défaut au spawn
derive_type <type> name <type2>;
derive_type { <types> } { <abilities> } <type3>;
```

## 2. Règles d'accès

```
allow <src> <dst>:ability {
    <modificateur>*
    <ability>:<args> …
};                               # le bloc se termine par « }; »
```

**Règle de ciblage** (apprise du compilateur, pas de la doc) : pour les abilities, `<dst>`
doit être `self` — *« type 'X' cannot target the abilities of another type ('Y') »*. Le
**graphe de privilège se lit dans les arguments des abilities de transition** :

```
settypeid:<type>          # ce type peut devenir <type>
gain_priv:<type>          # ce type peut hériter des privilèges de <type>
channel_connect:<type>    # ce type peut se connecter au canal de <type>
gain_priv: file:read:<type>   # permission de classe custom : classe, opération, type
```

Forme sur une ligne pour les canaux :
```
allow <src> <dst>:channel connect;
```

`allow_attach` et `allow_link` (chemins) :
```
allow_attach <type> <chemin> [<type_canal>];
allow_attach <type> { <règles> };
allow_link <type> <chemin>;
allow_link <type> { <règles> };
mount_point <uid>:<gid> { <règles> };
```

## 3. Ability à arguments

```
mem_phys:1024-4096,0x1200000-0x24000000   # plages hexadécimales et décimales
setuid:4,6,23                               # liste disjointe
setuid:4-6,23,96-                           # plages, bornes ouvertes (« 96- »)
settypeid:server1,server2                   # noms de types comme arguments
mem_phys:gpu_mem                            # référence à une plage nommée
able_create                                 # ability sans argument
```

## 4. Modificateurs (dans le bloc `{ … }`)

**Ce que le compilateur 8.0 accepte réellement** (vérifié sur le corpus) : `nonroot`,
`unlock`, `noinherit` — et eux seuls. Les autres jetons que `secpol_language.html` cite
comme modificateurs (`inherit`, `deny`, `allow`, `subrange`, `root`) sont **refusés** par
`secpolcompile` 8.0 avec `undefined ability <token>`. C'est le compilateur qui fait foi ;
le vocabulaire de Tamis (`core/src/secpol_abil.c`, `site/tamis-secpol-abil.js`) est aligné
sur ces trois-là.

> ⚠️ **Correction importante — j'avais tort précédemment.** Le binaire **QNX 7.1** utilisait
> les chaînes internes `no-inherit` (avec tiret) et `lock`. La documentation officielle
> **8.0** écrit **`noinherit`** (sans tiret) et **`unlock`**. La leçon, enregistrée : les
> chaînes internes d'un binaire d'exécution ne sont pas la syntaxe source. `docs/GRAMMAR.md`
> retient la **forme 8.0** ; `preuve/qnx71/syntaxe-attenstee.txt` documente l'écart 7.1.

## 5. Commentaires et include

- Les commentaires commencent par `#` (le `<!-- -->` n'est PAS accepté : le compilateur le
  rejette avec `invalid token '<!--'`).
- `include` est reconnu par l'outil (`mustbesur` pas attesté en production ici).

## 6. Ce que le compilateur vérifie déjà (autorité, à ne pas réinventer)

Capture réelle dans `preuve/qnx8/` :
- type non déclaré → `undefined type 'X'` (refus, 1 erreur)
- cibler les abilities d'un autre type → `type 'X' cannot target the abilities of another type ('Y')`
- ability inconnue → `undefined ability network/bind/privport`
- **avertissement de moindre privilège** : `warning: type X has unneeded gain_priv abilities: …`
  → c'est exactement le trouble que le lint de Tamis détecte à froid (TAM-06), mais le
  compilateur le voit, lui, **après** compilation.

`secpolcompile` (usage réel) : `secpolcompile [-em] [-p policy-id] [-o output-file] file …`,
dont `-e` = « Output as a secpolgenerate event file » — c'est la boucle des **fichiers
d'événements** dont parlait le cahier (simulation d'usages rares).

## 7. Statut E1/E2/E3

| Essai | Question | Réponse |
|---|---|---|
| E1 | Grammaire exacte ? | **OUI** — gelée dans ce fichier, croisée avec `secpolcompile` |
| E2 | `secpolcompile` s'exécute sur l'hôte ? | **OUI** — il a compilé `politique-qnx8.secpol` (0 erreur) |
| E3 | Politique d'exemple publique ? | **OUI** — `fixtures/politique-qnx8.secpol` (acceptée par l'outil officiel) |

**Fait.** Le parseur lit **ce langage** en C (`core/src/secpol*.c`) et en JavaScript
(`site/tamis-secpol*.js`), et le différentiel oracle compare l'acceptation de Tamis à celle
de `secpolcompile` sur les 42 politiques du corpus — accord total. Le format de fixture
provisoire (`.mpf`) reste accepté pour les démonstrations.