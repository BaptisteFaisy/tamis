/* SPDX-License-Identifier: MIT */
/* Tamis — tables des déclarations du parseur secpol : types, classes, permissions,
 * attributs, abilities dynamiques, plages nommées et relabel de canal.
 *
 * Extraites de secpol.c pour rester sous la barre des 500 lignes par fichier
 * (règle projet). Toutes les fonctions partagent le « parseur » (secpol_priv.h).
 */
#include "secpol_priv.h"
#include <string.h>

/* Vrai si `nom` est un type déclaré dans la politique. */
int est_type_declare(parseur *p, const char *nom)
{
    type_lien *t;
    for (t = p->types; t; t = t->suivant) {
        if (strcmp(t->nom, nom) == 0) { return 1; }
    }
    return 0;
}

/* Enregistre une ability dynamique déclarée (ability network/bind/privport;).
   -3 : l'ability est déjà déclarée (le compilateur la refuse). */
int ajouter_ability(parseur *p, char *nom)
{
    ability_lien *n;
    if (ability_declaree(p, nom)) { return -3; }
    n = tamis_arena_alloc(p->a, sizeof *n);
    if (!n) { return -2; }
    n->nom = nom;
    n->suivant = p->abilites;
    p->abilites = n;
    return 0;
}

int ability_declaree(parseur *p, const char *nom)
{
    ability_lien *a;
    for (a = p->abilites; a; a = a->suivant) {
        if (strcmp(a->nom, nom) == 0) { return 1; }
    }
    return 0;
}

/* Enregistre une plage nommée déclarée (range gpu_mem;). La re-déclaration est acceptée
   par le compilateur : aucun contrôle de doublon ici. */
int ajouter_range(parseur *p, char *nom)
{
    range_lien *r = tamis_arena_alloc(p->a, sizeof *r);
    if (!r) { return -2; }
    r->nom = nom;
    r->suivant = p->ranges;
    p->ranges = r;
    return 0;
}

int range_declaree(parseur *p, const char *nom)
{
    range_lien *r;
    for (r = p->ranges; r; r = r->suivant) {
        if (strcmp(r->nom, nom) == 0) { return 1; }
    }
    return 0;
}

int classe_declaree(parseur *p, const char *nom)
{
    classe_lien *c;
    for (c = p->classes; c; c = c->suivant) {
        if (strcmp(c->nom, nom) == 0) { return 1; }
    }
    return 0;
}

int ajouter_classe(parseur *p, char *nom)
{
    classe_lien *c = tamis_arena_alloc(p->a, sizeof *c);
    if (!c) { return -2; }
    if (classe_declaree(p, nom)) { return -3; } /* doublon */
    c->nom = nom;
    c->perms = NULL;
    c->n_perms = 0;
    c->suivant = p->classes;
    p->classes = c;
    return 0;
}

int permission_declaree(parseur *p, const char *classe, const char *perm)
{
    classe_lien *c;
    size_t i;
    for (c = p->classes; c; c = c->suivant) {
        if (strcmp(c->nom, classe) != 0) { continue; }
        for (i = 0; i < c->n_perms; i++) {
            if (strcmp(c->perms[i], perm) == 0) { return 1; }
        }
        return 0;
    }
    return 0;
}

int ajouter_permission(parseur *p, char *nom, char *perm)
{
    classe_lien *c;
    char **liste;
    for (c = p->classes; c; c = c->suivant) {
        if (strcmp(c->nom, nom) != 0) { continue; }
        liste = tamis_arena_alloc(p->a, (c->n_perms + 2) * sizeof(char *));
        if (!liste) { return -2; }
        if (c->perms) {
            memcpy(liste, c->perms, c->n_perms * sizeof(char *));
        }
        liste[c->n_perms] = perm;
        liste[c->n_perms + 1] = NULL;
        c->perms = liste;
        c->n_perms++;
        return 0;
    }
    return -1;
}

int attribut_declare(parseur *p, const char *nom)
{
    attr_lien *a;
    for (a = p->attrs; a; a = a->suivant) {
        if (strcmp(a->nom, nom) == 0) { return 1; }
    }
    return 0;
}

int ajouter_attribut(parseur *p, char *nom)
{
    attr_lien *a = tamis_arena_alloc(p->a, sizeof *a);
    if (!a) { return -2; }
    a->nom = nom;
    a->suivant = p->attrs;
    p->attrs = a;
    return 0;
}

/* Vrai si le terme est un type déclaré OU un attribut déclaré. */
int portee_attribut(parseur *p, const char *nom)
{
    return est_type_declare(p, nom) || attribut_declare(p, nom);
}

/*
 * Cohérence du relabel de canal : toutes les règles allow_attach d'un même type
 * doivent relabeler vers le MÊME type — ou aucune ne le fait. Mélanger « relabel »
 * et « pas de relabel » pour un type est refusé par le compilateur (I24).
 */
int ajouter_relabel(parseur *p, char *type, char *cible, long ligne, long colonne)
{
    relabel_lien *r;
    for (r = p->relabels; r; r = r->suivant) {
        if (strcmp(r->type, type) != 0) { continue; }
        if ((r->cible == NULL) == (cible == NULL) &&
            (!cible || strcmp(r->cible, cible) == 0)) {
            return 0; /* cohérent avec les règles déjà vues */
        }
        return echec(p, ligne, colonne,
                     "relabel de canal incohérent : toutes les règles de ce type "
                     "doivent relabeler vers le même type");
    }
    r = tamis_arena_alloc(p->a, sizeof *r);
    if (!r) { return -2; }
    r->type = type;
    r->cible = cible;
    r->suivant = p->relabels;
    p->relabels = r;
    return 0;
}