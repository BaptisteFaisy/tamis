<!-- SPDX-License-Identifier: MIT -->
# Checklist « prêt à soumettre » — 20 points binaires

**Vérification automatique :** `bash tools/check_soumission.sh`
(la version lisible de cette liste est dans le script ; les points 19 et 20 sont humains).

| # | Point | Vérifié par |
|---|---|---|
| 1 | Licence MIT visible | `LICENSE` |
| 2 | Le README cite le dépôt et le site | `README.md` |
| 3 | Le site est en ligne (HTTP 200) | `curl` |
| 4 | Les 7 onglets du site sont présents | `curl` + `grep` |
| 5 | Le formulaire Devpost a ses 13 sections | `docs/devpost-form.md` |
| 6 | Le codelab est présent et jouable | `docs/codelab/01-…` |
| 7 | La traçabilité des affirmations QNX existe | `docs/AFFIRMATIONS-QNX.md` |
| 8 | Le rétroplanning daté existe | `docs/RETROPLANNING.md` |
| 9 | Le gel de périmètre est respecté | `tools/check_perimetre.sh` |
| 10 | La fixture QNX 8.0 réelle est présente | `fixtures/politique-qnx8.secpol` |
| 11 | Les preuves QNX 7.1 **et** 8.0 sont publiées | `preuve/qnx71/`, `preuve/qnx8/` |
| 12 | Toute la suite de tests est verte | `make test` |
| 13 | La garde de taille est verte | `tests/garde.py` |
| 14 | Les mutants sont tous tués | `tests/mutants.py` |
| 15 | Aucun chemin absolu codé en dur | `grep` |
| 16 | Le budget CI tient en 5 min | `.github/workflows/ci.yml` |
| 17 | Le site est conforme à ses sources | `tools/build_site.py` |
| 18 | Aucune note « à faire » dans les documents publics | `grep` |
| 19 | **Vidéo 3 min (+ version 90 s)** | **humain** |
| 20 | **Soumission Devpost** | **humain** |

## Règle de jeu

Les 18 points automatiques sont **objectifs** : un échec est un blocage, pas un avis.
Les points 19 et 20 demandent le porteur du projet (caméra + compte Devpost).

## La veille de la soumission

```sh
bash tools/check_perimetre.sh      # périmètre gelé : 18/18
bash tools/check_soumission.sh      # les 20 points
make test                          # suite complète
```
Relire le dossier Devpost **à deux** (cf. `docs/AFFIRMATIONS-QNX.md`, section « relecture ») :
chaque affirmation VÉRIFIÉ doit pointer vers un artefact rejouable, et aucune formulation
interdite (`docs/AFFIRMATIONS-QNX.md`, section « ce que le dossier ne doit pas affirmer »)
ne doit subsister.