/* SPDX-License-Identifier: MIT */
/* Tamis — encodage SMT-LIB2 de l'accessibilité, pour prouver l'absence de chemin. */
/* Cœur pur : aucun appel à un solveur depuis core/ (l'appelant peut lancer z3 lui-même). */
#ifndef TAMIS_SMT_H
#define TAMIS_SMT_H

#include <stddef.h>
#include "tamis/model.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Émet l'encodage SMT-LIB2 de la question « src atteint-il dst ? » pour le modèle `m`.
 *
 * Encodage en clauses de Horn (décidable, rapide) :
 *   - r_i_j Booléen = « i atteint j »
 *   - r_i_i est vrai (réflexivité)
 *   - pour chaque arête k→j et chaque i : r_i_k ∧ e_k_j ⇒ r_i_j
 *   - on ajoute (not r_s_d) et on demande une modèle
 *     · UNSAT  → r_s_d est toujours vrai  → le chemin EXISTE
 *     · SAT    → un modèle où r_s_d est faux → le chemin N'EXISTE (preuve)
 *
 * Retourne le nombre d'octets qu'il faudrait (sémantique snprintf), ou <0 si l'index
 * de type est invalide. `buf` peut être NULL avec cap 0 pour mesurer la taille.
 */
long tamis_smt_reach(const tamis_model *m, int src, int dst, char *buf, size_t cap);

/*
 * Émet l'encodage de l'ensemble du modèle (tous les couples) — utile pour une preuve
 * globale d'inaccessibilité, et pour archiver la preuve hors ligne.
 */
long tamis_smt_all(const tamis_model *m, char *buf, size_t cap);

#ifdef __cplusplus
}
#endif
#endif /* TAMIS_SMT_H */
