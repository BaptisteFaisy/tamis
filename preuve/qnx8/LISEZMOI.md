<!-- SPDX-License-Identifier: MIT -->
# Preuve — QNX 8.0 : le compilateur officiel accepte une politique de Tamis

**Capturé le 12/11/2026** sur le serveur r630, avec le **SDP QNX 8.0 installé**
(`~/qnx800`, 14 Go, `host/linux/x86_64/usr/bin/secpolcompile`).

## Ce que ce dossier établit

Le différentiel oracle (TAM-12) est **réel, dans un sens** : Tamis produit une politique
(`fixtures/politique-qnx8.secpol`) que le compilateur officiel **accepte**.

| Fichier | Contenu | Preuve |
|---|---|---|
| `politique-qnx8.secpol` | politique réelle au format 8.0 (types, range, class, rules d'abilities, transitions `settypeid:`/`gain_priv:`, canal `connect`) | acceptée par `secpolcompile` |
| `compilation.txt` | `secpolcompile -o politique.bin` → **0 errors, 2 warnings** | le compilateur officiel compile la politique |
| `refus.txt` | politique invalide → `undefined type 'app_tiers'` (1 error) | le compilateur refuse les entrées invalides |
| `usage.txt` | `secpolcompile [-em] [-p policy-id] [-o output-file] file …` ; `-e` produit un **fichier d'événements secpolgenerate** | E2 : l'outil hôte tourne et documente la boucle d'événements |
| `politique.bin` | binaire compilé, 1 120 octets | l'artefact de sortie est produit |
| `empreintes.txt` | SHA-256 de `politique.bin` : `e990d0cd60faa06c6f0f08614554a8580090679b7993c133806e890dc38d0634` | reproductibilité |

## Ce qu'elle ne dit pas (honnêteté)

- **Le sens inverse du différentiel n'est pas encore fermé** : le parseur de Tamis ne lit
  PAS encore ce format (il lit le format de fixture provisoire `.mpf`). Tant que le parseur
  du vrai `secpol` n'est pas écrit, Tamis ne peut pas comparer son acceptation à celle de
  `secpolcompile` sur le même corpus. Ce dossier prouve que **la politique et la grammaire
  sont les bonnes** ; le parseur est la dernière brique.
- Les **2 warnings** du compilateur (`unneeded gain_priv abilities: file:read:file1_t,
  pathspace`) sont le signe que cette politique a des privilèges sur-dimensionnés : c'est
  exactement ce que le lint de Tamis détecte à froid (TAM-06), le compilateur, lui, ne le
  voit qu'après compilation. C'est déjà un échantillon du « trou S3 » exploité.

## Les trois faits de grammaire corrigés grâce au compilateur

1. **Commentaire** : `#`, et non `<!-- -->` (rejeté avec `invalid token '<!--'`).
2. **Ciblage** : la cible d'une règle d'abilities est `self` — cibler un autre type donne
   `type 'X' cannot target the abilities of another type ('Y')`. Le graphe de privilège
   s'exprime par les arguments `settypeid:` / `gain_priv:`.
3. **Modificateurs** : la doc 8.0 écrit `noinherit` (sans tiret) et `unlock`, alors que le
   binaire 7.1 utilisait `no-inherit` et `lock`. Voir `docs/GRAMMAR.md` §4.

## Reproduction

```sh
ssh r630
. ~/qnx800/qnxsdp-env.sh
secpolcompile -o politique.bin fixtures/politique-qnx8.secpol   # 0 erreur, 2 warnings
sha256sum politique.bin     # e990d0cd60faa06c6f0f08614554a8580090679b7993c133806e890dc38d0634
```