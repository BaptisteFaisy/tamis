/* SPDX-License-Identifier: MIT */
/* Tamis — parseur du langage de politique QNX 8.0 (secpol).
 * Contrat : docs/SECPOL-PARSEUR.md. Grammaire : docs/GRAMMAR.md.
 * Produit le MÊME tamis_model que le parseur provisoire : les moteurs ne changent pas. */
#ifndef TAMIS_SECPOL_H
#define TAMIS_SECPOL_H

#include <stddef.h>
#include "tamis/model.h"
#include "tamis/verdict.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Parse une politique secpol et remplit `out`.
 * Retour : 0 succès ; -1 erreur de syntaxe (*err rempli : code, ligne, colonne, cause) ;
 *          -2 arène épuisée.
 * Sous-ensemble STRICT : aucune analyse approximative (CONTRACT.md §1).
 */
int tamis_model_parse_secpol(const char *text, tamis_arena *a, tamis_model *out,
                              tamis_verdict *err);

#ifdef __cplusplus
}
#endif
#endif /* TAMIS_SECPOL_H */
