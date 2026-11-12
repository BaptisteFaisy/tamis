/* SPDX-License-Identifier: MIT */
/* Tamis — structures internes du parseur secpol (partagées entre le lexique et le parseur).
 * INTERNE : ne fait pas partie de l'API publique. */
#ifndef TAMIS_SECPOL_PRIV_H
#define TAMIS_SECPOL_PRIV_H

#include <stddef.h>
#include "tamis/model.h"
#include "tamis/verdict.h"

typedef enum { J_EOF, J_IDENT, J_NUM, J_HEX, J_PUNCT } nature;

typedef struct {
    nature  kind;
    char   *txt;
    long    ligne;
    long    colonne;
} jeton;

typedef struct {
    const char *p;
    const char *fin;
    long        ligne;
    long        colonne;
} lexique;

typedef struct type_lien {
    char *nom;
    long  ligne;
    struct type_lien *suivant;
} type_lien;

typedef struct regle_lien {
    char *src;
    char *dst;
    char *ability;
    long  ligne;
    int   type_obligatoire; /* 1 : la cible DOIT être un type déclaré ; 0 : ability seule */
    struct regle_lien *suivant;
} regle_lien;

typedef struct {
    lexique      lx;
    tamis_arena *a;
    tamis_model *m;
    tamis_verdict *err;
    type_lien   *types;
    regle_lien  *regles;
    long         n_types;
    long         n_regles;
    int          default_spawn;
} parseur;

/* Lexique (secpol_lex.c) */
void  lx_init(lexique *l, const char *texte);
void  lx_avance(lexique *l, size_t n);
void  lx_sauter(lexique *l);
int   lx_suivant(lexique *l, tamis_arena *a, jeton *j);
char *lx_brut(lexique *l, tamis_arena *a, const char *arretants, long *ligne, long *colonne);
char *lx_copier(tamis_arena *a, const char *debut, size_t n);

#endif /* TAMIS_SECPOL_PRIV_H */
