/* SPDX-License-Identifier: MIT */
/* Tamis — moteurs d'analyse. Chacun émet des verdicts depuis le modèle (graphe). */
/* API gelée : les moteurs sont indépendants et codent contre ces signatures. */
#ifndef TAMIS_ANALYZE_H
#define TAMIS_ANALYZE_H

#include "tamis/model.h"
#include "tamis/verdict.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Sortie : tableau de verdicts fourni par l'appelant + arène pour les chemins. */
typedef struct tamis_out {
    tamis_verdict *v;
    size_t         cap;
    size_t         n;       /* verdicts émis */
    tamis_arena   *arena;
    const char    *file;    /* nom du fichier analysé ; peut être NULL */
} tamis_out;

void tamis_out_init(tamis_out *o, tamis_verdict *v, size_t cap, tamis_arena *a, const char *file);
int  tamis_out_emit(tamis_out *o, const tamis_verdict *vd); /* 0 ok, -1 tableau plein */

/*
 * BFS de s à d (indices de types). Retour :
 *   1  chemin trouvé — path et rules alloués sur l'arène (noms de types, libellés de règles),
 *   0  aucun chemin — *visits = nombre de nœuds visités (c'est la PREUVE d'absence),
 *  <0  erreur (indice invalide, arène épuisée).
 * Implémentation unique, partagée par tamis_reach et tamis_proof (zéro duplication).
 */
int tamis_reach_path(const tamis_model *m, int s, int d, tamis_arena *a,
                     const char ***path, size_t *path_len,
                     const char ***rules, size_t *rules_len, size_t *visits);

/* Accessibilité (TAM-04) : émet TAMIS_PATH_FOUND (chemin complet + règles) si un chemin
   de src_name à dst_name existe. Retour : 1 chemin trouvé, 0 aucun, <0 erreur/type inconnu. */
int tamis_reach(tamis_out *o, const tamis_model *m, const char *src_name, const char *dst_name);

/* Preuve de non-interférence (TAM-05/20) : parcours exhaustif depuis src_name. Si dst_name
   atteignable -> TAMIS_INTERFERENCE_POSSIBLE (chemin). Sinon -> certificat TAMIS_OK
   « absence prouvée » avec le compteur de visites. Retour : 1 interférence, 0 prouvée saine, <0 erreur. */
int tamis_proof(tamis_out *o, const tamis_model *m, const char *src_name, const char *dst_name);

/* Explication pédagogique (idées 26/51/65) : émet le verdict d'accessibilité PUIS une
   explication en langage naturel — chaîne coupable + correctif minimal si chemin trouvé,
   ou périmètre de complétude + angles morts bornés (TAM-21) si aucun chemin.
   Retour : 1 chemin trouvé, 0 sain, <0 erreur. */
int tamis_explain(tamis_out *o, const tamis_model *m, const char *src_name, const char *dst_name);

/* Lint moindre privilège (TAM-06) : processus non typés + default_spawn_type manquant.
   Retour : nombre de verdicts émis, <0 erreur. */
int tamis_lint(tamis_out *o, const tamis_model *m);

/* Minimisation (TAM-07) : retire les règles redondantes (doublons src->dst) et prouve
   l'équivalence (accessibilité toute-paire inchangée). Émet TAMIS_REDUNDANT_RULE par doublon
   puis un verdict de preuve d'équivalence. Retour : nb verdicts, <0 erreur. */
int tamis_minimize(tamis_out *o, const tamis_model *m);

/* Diff sémantique (TAM-08) : de base vers cand. Émet TAMIS_PRIVILEGE_WIDENED par arête
   ajoutée, TAMIS_PRIVILEGE_NARROWED par arête retirée. Retour : nb verdicts, <0 erreur. */
int tamis_diff(tamis_out *o, const tamis_model *base, const tamis_model *cand);

#ifdef __cplusplus
}
#endif
#endif /* TAMIS_ANALYZE_H */
