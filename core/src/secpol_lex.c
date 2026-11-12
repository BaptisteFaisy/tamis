/* SPDX-License-Identifier: MIT */
/* Tamis — lexique du langage secpol (QNX 8.0). Voir docs/SECPOL-PARSEUR.md.
 *
 * Le découpage distingue les identifiants, les nombres décimaux et hexadécimaux, et la
 * ponctuation `;{}:`. Les chemins et les abilities composées (network/bind/privport) sont lus
 * en mode BRUT (lx_brut), car ils ne suivent pas le découpageusual des mots.
 */
#include "secpol_priv.h"
#include <string.h>

void lx_init(lexique *l, const char *texte)
{
    l->p = texte;
    l->fin = texte + strlen(texte);
    l->ligne = 1;
    l->colonne = 1;
}

void lx_avance(lexique *l, size_t n)
{
    size_t i;
    for (i = 0; i < n && l->p < l->fin; i++, l->p++) {
        if (*l->p == '\n') {
            l->ligne++;
            l->colonne = 1;
        } else {
            l->colonne++;
        }
    }
}

static int est_blanc(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

void lx_sauter(lexique *l)
{
    while (l->p < l->fin) {
        if (est_blanc(*l->p)) {
            lx_avance(l, 1);
        } else if (*l->p == '#') {
            while (l->p < l->fin && *l->p != '\n') {
                lx_avance(l, 1);
            }
        } else {
            return;
        }
    }
}

static int est_debut_ident(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int est_ident(char c)
{
    return est_debut_ident(c) || (c >= '0' && c <= '9');
}

static int est_chiffre(char c) { return c >= '0' && c <= '9'; }

char *lx_copier(tamis_arena *a, const char *debut, size_t n)
{
    char *s = tamis_arena_strdup(a, debut, n);
    return s ? s : (char *)"";
}

int lx_suivant(lexique *l, tamis_arena *a, jeton *j)
{
    const char *debut;
    size_t n;
    lx_sauter(l);
    j->ligne = l->ligne;
    j->colonne = l->colonne;
    j->txt = NULL;
    if (l->p >= l->fin) {
        j->kind = J_EOF;
        j->txt = lx_copier(a, "", 0);
        return 0;
    }
    debut = l->p;
    if (est_debut_ident(*l->p)) {
        while (l->p < l->fin && est_ident(*l->p)) { lx_avance(l, 1); }
        n = (size_t)(l->p - debut);
        j->kind = J_IDENT;
    } else if (est_chiffre(*l->p)) {
        if (l->p + 1 < l->fin && l->p[0] == '0' && (l->p[1] == 'x' || l->p[1] == 'X')) {
            lx_avance(l, 2);
            while (l->p < l->fin && ((*l->p >= '0' && *l->p <= '9') ||
                                      (*l->p >= 'a' && *l->p <= 'f') ||
                                      (*l->p >= 'A' && *l->p <= 'F'))) { lx_avance(l, 1); }
            j->kind = J_HEX;
        } else {
            while (l->p < l->fin && est_chiffre(*l->p)) { lx_avance(l, 1); }
            j->kind = J_NUM;
        }
        n = (size_t)(l->p - debut);
    } else if (*l->p == ';' || *l->p == '{' || *l->p == '}' || *l->p == ':') {
        lx_avance(l, 1);
        n = 1;
        j->kind = J_PUNCT;
    } else {
        return -1;
    }
    j->txt = lx_copier(a, debut, n);
    return 0;
}

char *lx_brut(lexique *l, tamis_arena *a, const char *arretants, long *ligne, long *colonne)
{
    const char *debut;
    lx_sauter(l);
    debut = l->p;
    while (l->p < l->fin && !strchr(arretants, *l->p)) { lx_avance(l, 1); }
    if (ligne) { *ligne = l->ligne; }
    if (colonne) { *colonne = l->colonne; }
    return lx_copier(a, debut, (size_t)(l->p - debut));
}
