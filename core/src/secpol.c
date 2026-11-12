/* SPDX-License-Identifier: MIT */
/* Tamis — parseur du langage de politique de sécurité QNX 8.0 (secpol).
 *
 * Contrat : docs/SECPOL-PARSEUR.md (gelé). Grammaire : docs/GRAMMAR.md (gelée depuis la
 * documentation officielle du SDP 8.0, croisée avec le compilateur `secpolcompile`).
 *
 * Sous-ensemble STRICT : toute production non listée produit TAMIS_PARSE_ERROR avec ligne
 * et colonne. Jamais d'analyse approximative (CONTRACT.md §1).
 */
#include "tamis/secpol.h"
#include "secpol_priv.h"
#include <string.h>

/* ————————————————————— erreurs et aides ————————————————————— */

static int echec(parseur *p, long ligne, long colonne, const char *cause)
{
    memset(p->err, 0, sizeof *p->err);
    p->err->code = TAMIS_PARSE_ERROR;
    p->err->sev = TAMIS_SEV_ERROR;
    p->err->line = ligne;
    p->err->column = colonne;
    p->err->message = cause;
    return -1;
}

static int attendre_pt(parseur *p, char c)
{
    jeton j;
    const char *msg = "jeton inattendu";
    if (c == ';') { msg = "« ; » attendu"; }
    else if (c == '{') { msg = "« { » attendu"; }
    else if (c == '}') { msg = "« } » attendu"; }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind == J_EOF) { return echec(p, j.ligne, j.colonne, "fin de fichier inattendue"); }
    if (j.kind != J_PUNCT || j.txt[0] != c) { return echec(p, j.ligne, j.colonne, msg); }
    return 0;
}

static int attendre_ident(parseur *p, char **sortie, long *ligne, long *colonne)
{
    jeton j;
    if (lx_suivant(&p->lx, p->a, &j) != 0) {
        return echec(p, p->lx.ligne, p->lx.colonne, "caractère inattendu");
    }
    if (j.kind != J_IDENT) { return echec(p, j.ligne, j.colonne, "identifiant attendu"); }
    if (sortie) { *sortie = j.txt; }
    if (ligne) { *ligne = j.ligne; }
    if (colonne) { *colonne = j.colonne; }
    return 0;
}

/* ————————————————————— construction des listes ————————————————————— */

static int ajouter_type(parseur *p, char *nom, long ligne)
{
    type_lien *t;
    for (t = p->types; t; t = t->suivant) {
        if (strcmp(t->nom, nom) == 0) { return echec(p, ligne, 1, "type déjà déclaré"); }
    }
    t = tamis_arena_alloc(p->a, sizeof *t);
    if (!t) { return -2; }
    t->nom = nom;
    t->ligne = ligne;
    t->suivant = p->types;
    p->types = t;
    p->n_types++;
    return 0;
}

static int ajouter_regle(parseur *p, char *src, char *dst, char *ability, long ligne, int obligatoire)
{
    regle_lien *r = tamis_arena_alloc(p->a, sizeof *r);
    if (!r) { return -2; }
    r->src = src;
    r->dst = dst;
    r->ability = ability;
    r->ligne = ligne;
    r->type_obligatoire = obligatoire;
    r->suivant = p->regles;
    p->regles = r;
    p->n_regles++;
    return 0;
}

static int est_transition(const char *mot)
{
    return strcmp(mot, "settypeid") == 0 || strcmp(mot, "gain_priv") == 0;
}

/* ————————————————————— productions ————————————————————— */

/* Un élément d'ability : « mot » ou « mot:args ». Les transitions (settypeid, gain_priv)
   portent le graphe de privilège ; les autres ne portent qu'une ability. */
static int p_ability_item(parseur *p, char *src, long ligne_regle)
{
    long lg = 0, cl = 0, al = 0, ac = 0;
    char *mot = lx_brut(&p->lx, p->a, ": \t\r\n;}", &lg, &cl);
    if (mot[0] == '\0') {
        return echec(p, lg, cl, "ability vide");
    }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == ':') {
        lx_avance(&p->lx, 1);
        for (;;) {
            char *arg = lx_brut(&p->lx, p->a, ", \t\r\n;}", &al, &ac);
            if (arg[0] == '\0') {
                return echec(p, al, ac, "argument d'ability manquant");
            }
            if (est_transition(mot)) {
                /* settypeid : la cible DOIT être un type déclaré ; gain_priv : si la cible
                   n'est pas un type, c'est une ability (le compilateur n'en fait qu'un warning). */
                int rc = ajouter_regle(p, src, arg, mot, ligne_regle,
                                        strcmp(mot, "settypeid") == 0);
                if (rc != 0) { return rc; }
            }
            lx_sauter(&p->lx);
            if (p->lx.p < p->lx.fin && *p->lx.p == ',') {
                lx_avance(&p->lx, 1);
                continue;
            }
            break;
        }
    }
    /* Un point-virgule peut terminer l'item ; la doc officielle ne le met pas, mais
       l'accepter évite de rejeter des politiques écrites autrement. */
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == ';') {
        lx_avance(&p->lx, 1);
    }
    return 0;
}

/* Bloc d'abilities entre accolades (déjà après le '{'). */
static int p_bloc_abilities(parseur *p, char *src, long ligne_regle)
{
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p >= p->lx.fin) {
            return echec(p, p->lx.ligne, p->lx.colonne, "accolade fermante manquante");
        }
        if (*p->lx.p == '}') { lx_avance(&p->lx, 1); return 0; }
        if (p_ability_item(p, src, ligne_regle) != 0) { return -1; }
    }
}

/* Saute un bloc { … } (imbrication comprise) ; le curseur est après l'accolade fermante. */
static int p_sauter_bloc(parseur *p)
{
    jeton j;
    int prof = 1;
    while (prof > 0) {
        if (lx_suivant(&p->lx, p->a, &j) != 0) {
            return echec(p, p->lx.ligne, p->lx.colonne, "caractère inattendu");
        }
        if (j.kind == J_EOF) {
            return echec(p, j.ligne, j.colonne, "accolade fermante manquante");
        }
        if (j.kind == J_PUNCT && j.txt[0] == '{') { prof++; }
        else if (j.kind == J_PUNCT && j.txt[0] == '}') { prof--; }
    }
    return 0;
}

static int p_allow(parseur *p)
{
    jeton j;
    char *src = NULL, *dst = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &src, &lg, &cl) != 0) { return -1; }
    if (attendre_ident(p, &dst, &lg, &cl) != 0) { return -1; }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != ':') {
        return echec(p, j.ligne, j.colonne, "« : » attendu après les deux types");
    }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_IDENT) { return echec(p, j.ligne, j.colonne, "mot-clé attendu après « : »"); }
    if (strcmp(j.txt, "ability") == 0) {
        int rc;
        if (strcmp(dst, "self") != 0) {
            return echec(p, lg, cl, "une règle d'abilities doit cibler « self »");
        }
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != '{') {
            return echec(p, j.ligne, j.colonne, "« { » attendu");
        }
        rc = p_bloc_abilities(p, src, lg);
        if (rc != 0) { return rc; }
        return attendre_pt(p, ';');
    }
    if (strcmp(j.txt, "channel") == 0) {
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_IDENT || strcmp(j.txt, "connect") != 0) {
            return echec(p, j.ligne, j.colonne, "« connect » attendu après « channel »");
        }
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != ';') {
            return echec(p, j.ligne, j.colonne, "« ; » attendu");
        }
        return ajouter_regle(p, src, dst, "channel", lg, 1);
    }
    return echec(p, j.ligne, j.colonne, "« ability » ou « channel » attendu");
}

/* allow_attach <type> <chemin> [<type>] ;  |  allow_attach <type> { bloc } ; */
static int p_allow_attach(parseur *p)
{
    char *type = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &type, &lg, &cl) != 0) { return -1; }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }
        return attendre_pt(p, ';');
    }
    lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p != ';') {
        lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);
    }
    return attendre_pt(p, ';');
}

/* allow_link <type> <chemin> ;  |  allow_link <type> { bloc } ; */
static int p_allow_link(parseur *p)
{
    char *type = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &type, &lg, &cl) != 0) { return -1; }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }
        return attendre_pt(p, ';');
    }
    lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);
    return attendre_pt(p, ';');
}

/* class <nom> { permission… } ; */
static int p_class(parseur *p)
{
    jeton j;
    char *nom = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &nom, &lg, &cl) != 0) { return -1; }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != '{') {
        return echec(p, j.ligne, j.colonne, "« { » attendu");
    }
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p >= p->lx.fin) {
            return echec(p, p->lx.ligne, p->lx.colonne, "accolade fermante manquante");
        }
        if (*p->lx.p == '}') { lx_avance(&p->lx, 1); break; }
        if (lx_suivant(&p->lx, p->a, &j) != 0 || j.kind != J_IDENT) {
            return echec(p, p->lx.ligne, p->lx.colonne, "permission attendue");
        }
    }
    return attendre_pt(p, ';');
}

/* derive_type A name B ;  |  derive_type { types } { abilities } C ; */
static int p_derive_type(parseur *p)
{
    jeton j;
    long lg = 0, cl = 0;
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }   /* les types */
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != '{') {
            return echec(p, j.ligne, j.colonne, "« { » attendu");
        }
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }   /* les abilities */
        if (attendre_ident(p, NULL, NULL, NULL) != 0) { return -1; }
        return attendre_pt(p, ';');
    }
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    return attendre_pt(p, ';');
}

/* default_spawn_type A B ;  (le nombre de types est libre) */
static int p_default_spawn(parseur *p)
{
    long lg = 0, cl = 0;
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p < p->lx.fin && *p->lx.p == ';') { break; }
        if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    }
    p->default_spawn = 1;
    return attendre_pt(p, ';');
}

/* mount_point uid:gid { bloc } ; */
static int p_mount_point(parseur *p)
{
    jeton j;
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_NUM && j.kind != J_HEX) {
        return echec(p, j.ligne, j.colonne, "uid attendu");
    }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != ':') {
        return echec(p, j.ligne, j.colonne, "« : » attendu");
    }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_NUM && j.kind != J_HEX) {
        return echec(p, j.ligne, j.colonne, "gid attendu");
    }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != '{') {
        return echec(p, j.ligne, j.colonne, "« { » attendu");
    }
    if (p_sauter_bloc(p) != 0) { return -1; }
    return attendre_pt(p, ';');
}

/* ————————————————————— construction du modèle ————————————————————— */

static int index_de(const char **noms, size_t n, const char *nom)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (strcmp(noms[i], nom) == 0) { return (int)i; }
    }
    return -1;
}

/* Remplit le modèle : types et règles dans l'ORDRE SOURCE (contrat du parseur §4). */
static int construire(parseur *p)
{
    size_t n = (size_t)p->n_types, k;
    const char **noms;
    type_lien *t;
    regle_lien *r;
    noms = tamis_arena_alloc(p->a, (n ? n : 1) * sizeof(char *));
    if (!noms) { return -2; }
    k = n;
    for (t = p->types; t; t = t->suivant) { k--; noms[k] = t->nom; }
    p->m->type_names = noms;
    p->m->n_types = n;
    p->m->rules = tamis_arena_alloc(p->a, (p->n_regles ? (size_t)p->n_regles : 1) * sizeof(tamis_rule));
    if (!p->m->rules) { return -2; }
    /* 1re passe : combien de règles sont conservées (une ability sans type n'en est pas une) */
    {
        size_t gardees = 0;
        for (r = p->regles; r; r = r->suivant) {
            if (index_de(noms, n, r->src) < 0) {
                return echec(p, r->ligne, 1, "type source non déclaré");
            }
            if (index_de(noms, n, r->dst) < 0 && r->type_obligatoire) {
                return echec(p, r->ligne, 1, "type cible non déclaré");
            }
            if (index_de(noms, n, r->dst) >= 0) {
                gardees++;
            }
        }
        k = gardees;
        for (r = p->regles; r; r = r->suivant) {
            int si = index_de(noms, n, r->src);
            int di = index_de(noms, n, r->dst);
            if (di < 0) {
                continue; /* ability sans type : pas d'arête */
            }
            k--;
            p->m->rules[k].src = si;
            p->m->rules[k].dst = di;
            p->m->rules[k].ability = r->ability;
            p->m->rules[k].id = NULL;
            p->m->rules[k].line = r->ligne;
        }
        p->m->n_rules = gardees;
    }
    p->m->has_default_spawn = p->default_spawn;
    return 0;
}

/* ————————————————————— pilote ————————————————————— */

static int analyser(parseur *p)
{
    jeton j;
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p >= p->lx.fin) { return 0; }
        if (lx_suivant(&p->lx, p->a, &j) != 0) {
            return echec(p, p->lx.ligne, p->lx.colonne, "caractère inattendu");
        }
        if (j.kind != J_IDENT) {
            return echec(p, j.ligne, j.colonne, "mot-clé de production attendu");
        }
        if (strcmp(j.txt, "type") == 0) {
            char *nom = NULL;
            long lg = 0, cl = 0;
            int rc;
            if (attendre_ident(p, &nom, &lg, &cl) != 0) { return -1; }
            rc = ajouter_type(p, nom, lg);
            if (rc != 0) { return rc; }
            if (attendre_pt(p, ';') != 0) { return -1; }
        } else if (strcmp(j.txt, "range") == 0 || strcmp(j.txt, "attribute") == 0) {
            if (attendre_ident(p, NULL, NULL, NULL) != 0) { return -1; }
            if (attendre_pt(p, ';') != 0) { return -1; }
        } else if (strcmp(j.txt, "class") == 0) {
            if (p_class(p) != 0) { return -1; }
        } else if (strcmp(j.txt, "allow") == 0) {
            if (p_allow(p) != 0) { return -1; }
        } else if (strcmp(j.txt, "allow_attach") == 0) {
            if (p_allow_attach(p) != 0) { return -1; }
        } else if (strcmp(j.txt, "allow_link") == 0) {
            if (p_allow_link(p) != 0) { return -1; }
        } else if (strcmp(j.txt, "derive_type") == 0) {
            if (p_derive_type(p) != 0) { return -1; }
        } else if (strcmp(j.txt, "default_spawn_type") == 0) {
            if (p_default_spawn(p) != 0) { return -1; }
        } else if (strcmp(j.txt, "mount_point") == 0) {
            if (p_mount_point(p) != 0) { return -1; }
        } else {
            return echec(p, j.ligne, j.colonne, "mot-clé de production inconnu");
        }
    }
}

int tamis_model_parse_secpol(const char *text, tamis_arena *a, tamis_model *out,
                              tamis_verdict *err)
{
    parseur p;
    int rc;
    if (!text || !a || !out || !err) { return -1; }
    memset(&p, 0, sizeof p);
    lx_init(&p.lx, text);
    p.a = a;
    p.m = out;
    p.err = err;
    memset(out, 0, sizeof *out);
    rc = analyser(&p);
    if (rc != 0) { return rc; }
    return construire(&p);
}
