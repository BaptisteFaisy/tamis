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

int echec(parseur *p, long ligne, long colonne, const char *cause)
{
    memset(p->err, 0, sizeof *p->err);
    p->err->code = TAMIS_PARSE_ERROR;
    p->err->sev = TAMIS_SEV_ERROR;
    p->err->line = ligne;
    p->err->column = colonne;
    p->err->message = cause;
    return -1;
}

int attendre_pt(parseur *p, char c)
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

int attendre_ident(parseur *p, char **sortie, long *ligne, long *colonne)
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

int ajouter_type(parseur *p, char *nom, long ligne)
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

int ajouter_regle(parseur *p, char *src, char *dst, char *ability, long ligne, int obligatoire)
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

int est_transition(const char *mot)
{
    return strcmp(mot, "settypeid") == 0 || strcmp(mot, "gain_priv") == 0;
}

/* Enregistre « <membre> est membre de l'attribut <attr> » (pour type X, attr;). */
int ajouter_membre_attr(parseur *p, char *attr, char *membre)
{
    membre_attr *m = tamis_arena_alloc(p->a, sizeof *m);
    if (!m) { return -2; }
    m->attr = attr;
    m->membre = membre;
    m->suivant = p->attributs;
    p->attributs = m;
    return 0;
}

/* Enregistre une ability dynamique déclarée (ability network/bind/privport;). */
int ajouter_ability(parseur *p, char *nom)
{
    ability_lien *n = tamis_arena_alloc(p->a, sizeof *n);
    if (!n) { return -2; }
    n->nom = nom;
    n->suivant = p->abilites;
    p->abilites = n;
    return 0;
}

/* type <nom> [ , <attribut> … ] ; */
static int p_type(parseur *p)
{
    char *nom = NULL;
    long lg = 0, cl = 0;
    int rc;
    if (attendre_ident(p, &nom, &lg, &cl) != 0) { return -1; }
    rc = ajouter_type(p, nom, lg);
    if (rc != 0) { return rc; }
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p >= p->lx.fin || *p->lx.p != ',') { break; }
        lx_avance(&p->lx, 1);       /* la virgule */
        char *attr = NULL;
        if (attendre_ident(p, &attr, &lg, &cl) != 0) { return -1; }
        rc = ajouter_membre_attr(p, attr, nom);
        if (rc != 0) { return rc; }
    }
    return attendre_pt(p, ';');
}

/* ability <nom> ;  — déclaration d'ability dynamique (le nom peut contenir « / »). */
static int p_ability_decl(parseur *p)
{
    char *nom = lx_brut(&p->lx, p->a, " \t\r\n;", NULL, NULL);
    if (nom[0] == '\0') {
        return echec(p, p->lx.ligne, p->lx.colonne, "nom d'ability attendu");
    }
    if (ajouter_ability(p, nom) != 0) { return -2; }
    return attendre_pt(p, ';');
}

/* ————————————————————— productions ————————————————————— */

/* ————————————————————— construction du modèle ————————————————————— */

static int index_de(const char **noms, size_t n, const char *nom)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (strcmp(noms[i], nom) == 0) { return (int)i; }
    }
    return -1;
}

static int est_type_declare(parseur *p, const char *nom)
{
    type_lien *t;
    for (t = p->types; t; t = t->suivant) {
        if (strcmp(t->nom, nom) == 0) { return 1; }
    }
    return 0;
}

/* Résout un terme (type ou attribut) en liste de noms de types (terminée par NULL). */
static const char **resoudre(parseur *p, const char *terme, size_t *n)
{
    const char **liste;
    membre_attr *m;
    size_t k = 0, cnt = 0;
    if (est_type_declare(p, terme)) {
        liste = tamis_arena_alloc(p->a, 2 * sizeof(char *));
        if (!liste) { return NULL; }
        liste[0] = terme; liste[1] = NULL;
        *n = 1;
        return liste;
    }
    for (m = p->attributs; m; m = m->suivant) {
        if (strcmp(m->attr, terme) == 0) { cnt++; }
    }
    if (cnt == 0) { return NULL; }
    liste = tamis_arena_alloc(p->a, (cnt + 1) * sizeof(char *));
    if (!liste) { return NULL; }
    for (m = p->attributs; m; m = m->suivant) {
        if (strcmp(m->attr, terme) == 0) { liste[k++] = m->membre; }
    }
    liste[k] = NULL;
    *n = k;
    return liste;
}

/* Remplit le modèle : types et règles dans l'ORDRE SOURCE (contrat du parseur §4). */
/* Résout les termes (types + attributs) en paires (src, dst, ability, ligne). */
static int resoudre_regles(parseur *p, paire_regle **paires_out, size_t *npaires_out)
{
    size_t npaires = 0;
    regle_lien *r;
    paire_regle *paires;
    paires = tamis_arena_alloc(p->a, (p->n_regles ? (size_t)p->n_regles : 1) * sizeof *paires);
    if (!paires) { return -2; }
    for (r = p->regles; r; r = r->suivant) {
        size_t ns = 0, nd = 0, u, v;
        const char **srcs = resoudre(p, r->src, &ns);
        const char **dsts;
        if (!srcs) {
            return echec(p, r->ligne, 1, "type source non déclaré");
        }
        dsts = resoudre(p, r->dst, &nd);
        if (!dsts) {
            if (r->type_obligatoire) {
                return echec(p, r->ligne, 1, "type cible non déclaré");
            }
            continue; /* ability sans type : pas d'arête */
        }
        for (u = 0; u < ns; u++) {
            for (v = 0; v < nd; v++) {
                paires[npaires].src = srcs[u];
                paires[npaires].dst = dsts[v];
                paires[npaires].ability = r->ability;
                paires[npaires].ligne = r->ligne;
                npaires++;
            }
        }
    }
    *paires_out = paires;
    *npaires_out = npaires;
    return 0;
}

/* Remplit le tableau de règles du modèle, dans l'ORDRE SOURCE. */
static int remplir_regles(parseur *p, const char **noms, size_t n)
{
    paire_regle *paires = NULL;
    size_t npaires = 0, i, k = 0;
    int rc = resoudre_regles(p, &paires, &npaires);
    if (rc != 0) { return rc; }
    for (i = npaires; i > 0; i--) { /* les paires sont dans l'ordre inverse de la source */
        paire_regle *pr = &paires[i - 1];
        int si = index_de(noms, n, pr->src);
        int di = index_de(noms, n, pr->dst);
        if (si < 0 || di < 0) { continue; }
        p->m->rules[k].src = si;
        p->m->rules[k].dst = di;
        p->m->rules[k].ability = pr->ability;
        p->m->rules[k].id = NULL;
        p->m->rules[k].line = pr->ligne;
        k++;
    }
    p->m->n_rules = k;
    return 0;
}

static int construire(parseur *p)
{
    size_t n = (size_t)p->n_types, k;
    const char **noms;
    type_lien *t;
    int rc;
    noms = tamis_arena_alloc(p->a, (n ? n : 1) * sizeof(char *));
    if (!noms) { return -2; }
    k = n;
    for (t = p->types; t; t = t->suivant) { k--; noms[k] = t->nom; }
    p->m->type_names = noms;
    p->m->n_types = n;
    p->m->rules = tamis_arena_alloc(p->a, (p->n_regles ? (size_t)p->n_regles : 1) * sizeof(tamis_rule));
    if (!p->m->rules) { return -2; }
    rc = remplir_regles(p, noms, n);
    if (rc != 0) { return rc; }
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
            if (p_type(p) != 0) { return -1; }
        } else if (strcmp(j.txt, "ability") == 0) {
            if (p_ability_decl(p) != 0) { return -1; }
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
