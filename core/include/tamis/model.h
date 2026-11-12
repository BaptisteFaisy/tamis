/* SPDX-License-Identifier: MIT */
/* Tamis — modèle en mémoire d'une politique : graphe orienté des types et règles. */
/* Le parseur secpol (lot L1, bloqué sur la grammaire) produira ce même modèle ; en */
/* attendant il est construit depuis un format de fixture provisoire (voir model.c). */
/* Cœur pur : aucune allocation — l'appelant fournit une arène (CONTRACT §2, ADR D-002). */
#ifndef TAMIS_MODEL_H
#define TAMIS_MODEL_H

#include <stddef.h>
#include "tamis/verdict.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Arène : allocation par bump sur un tampon fourni par l'appelant. Zéro malloc. */
typedef struct tamis_arena {
    char  *base;
    size_t cap;
    size_t used;
} tamis_arena;

void  tamis_arena_init(tamis_arena *a, char *buf, size_t cap);
void *tamis_arena_alloc(tamis_arena *a, size_t n); /* aligné sur 8 ; NULL si épuisée */
char *tamis_arena_strdup(tamis_arena *a, const char *s, size_t len);

/* Une règle = arête orientée src -> dst (un accès autorisé de src vers dst). */
typedef struct tamis_rule {
    int         src;     /* index du type source dans model->type_names */
    int         dst;     /* index du type cible */
    const char *ability; /* étiquette (ability/canal) ; NULL si non spécifiée */
    const char *id;      /* identifiant de règle ; NULL sinon */
    long        line;    /* ligne source (1-based) */
} tamis_rule;

/* Un processus sans type (lint TAM-06). */
typedef struct tamis_untyped {
    const char *name;
    long        line;
} tamis_untyped;

/* Modèle construit : tous les pointeurs référencent l'arène de construction. */
typedef struct tamis_model {
    const char   **type_names;  /* noms des types (nœuds) */
    size_t          n_types;
    tamis_rule     *rules;      /* arêtes orientées */
    size_t          n_rules;
    tamis_untyped  *untyped;    /* processus sans type (lint) */
    size_t          n_untyped;
    int             has_default_spawn; /* 1 si default_spawn_type déclaré */
} tamis_model;

/*
 * Construit un modèle depuis le texte (format provisoire documenté dans model.c).
 * Tout le stockage est pris sur l'arène `a`. Retour :
 *   0  : succès, `out` rempli.
 *  -1  : erreur de syntaxe — `err` rempli (TAMIS_PARSE_ERROR, ligne/colonne, cause).
 *  -2  : arène épuisée (capacité insuffisante).
 * Aucune analyse approximative : la première erreur arrête (CONTRACT §1).
 */
int tamis_model_parse(const char *text, tamis_arena *a, tamis_model *out,
                      tamis_verdict *err);

/* Index d'un type par son nom, ou -1. */
int tamis_model_type_index(const tamis_model *m, const char *name);

#ifdef __cplusplus
}
#endif
#endif /* TAMIS_MODEL_H */
