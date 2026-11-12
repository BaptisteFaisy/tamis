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


/* Un « repère » de règle : un nom, ou un groupe { n1 n2 }. */
typedef struct nom_lien {
    char *nom;
    struct nom_lien *suivant;
} nom_lien;

/* Une arête résolue : source, cible, libellé d'ability, ligne. */
typedef struct {
    const char *src;
    const char *dst;
    const char *ability;
    long        ligne;
} paire_regle;

/* Une appartenance « type <membre> est membre de l'attribut <attr> » (type X, attr;). */
typedef struct membre_attr {
    char *attr;
    char *membre;
    struct membre_attr *suivant;
} membre_attr;

/* Une ability dynamique déclarée (ability network/bind/privport;). */
typedef struct ability_lien {
    char *nom;
    struct ability_lien *suivant;
} ability_lien;

typedef struct {
    lexique      lx;
    tamis_arena *a;
    tamis_model *m;
    tamis_verdict *err;
    type_lien   *types;
    regle_lien  *regles;
    membre_attr *attributs;
    ability_lien *abilites;
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


/* — fonctions partagées entre secpol.c et secpol_parse.c — */
int  echec(parseur *p, long ligne, long colonne, const char *cause);
int  attendre_ident(parseur *p, char **sortie, long *ligne, long *colonne);
int  attendre_pt(parseur *p, char c);
int  ajouter_type(parseur *p, char *nom, long ligne);
int  ajouter_regle(parseur *p, char *src, char *dst, char *ability, long ligne, int obligatoire);
int  ajouter_membre_attr(parseur *p, char *attr, char *membre);
int  ajouter_ability(parseur *p, char *nom);
int  est_transition(const char *mot);
int  p_ability_item(parseur *p, char *src, long ligne_regle);
int  p_bloc_abilities(parseur *p, char *src, long ligne_regle);
int  p_sauter_bloc(parseur *p);
int  p_class(parseur *p);
int  p_allow(parseur *p);
int  p_allow_attach(parseur *p);
int  p_allow_link(parseur *p);
int  p_derive_type(parseur *p);
int  p_default_spawn(parseur *p);
int  p_mount_point(parseur *p);

#endif /* TAMIS_SECPOL_PRIV_H */
