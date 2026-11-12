/* SPDX-License-Identifier: MIT */
/* Tamis — arène + construction du modèle depuis le format de fixture provisoire. */
/*
 * FORMAT PROVISOIRE (une directive par ligne, '#' = commentaire) — À REMPLACER par le
 * parseur secpol (lot L1) dès la grammaire gelée (docs/GRAMMAR.md) :
 *   type <NAME>                             déclare un type (nœud du graphe)
 *   allow <SRC> <DST> [<ABILITY>] [@<RID>]  arête orientée SRC -> DST
 *   untyped <NAME>                          processus sans type (lint)
 *   default_spawn                           marque la présence de default_spawn_type
 * NAME/SRC/DST/ABILITY = identifiants [A-Za-z0-9_]. Toute autre ligne = TAMIS_PARSE_ERROR.
 */
#include "tamis/model.h"
#include <ctype.h>
#include <string.h>

void tamis_arena_init(tamis_arena *a, char *buf, size_t cap)
{
    a->base = buf; a->cap = cap; a->used = 0;
}

void *tamis_arena_alloc(tamis_arena *a, size_t n)
{
    size_t aligned = (n + 7u) & ~(size_t)7u;
    void *p;
    if (a->used + aligned > a->cap || aligned < n) {
        return NULL;
    }
    p = a->base + a->used;
    a->used += aligned;
    return p;
}

char *tamis_arena_strdup(tamis_arena *a, const char *s, size_t len)
{
    char *p = (char *)tamis_arena_alloc(a, len + 1);
    if (!p) {
        return NULL;
    }
    memcpy(p, s, len);
    p[len] = '\0';
    return p;
}

/* Itérateur de lignes : évite de dupliquer la boucle de parcours entre passes. */
typedef struct { const char *p; const char *end; long line; } line_iter;

static void li_init(line_iter *it, const char *text)
{
    it->p = text; it->end = text + strlen(text); it->line = 0;
}

static const char *li_next(line_iter *it, const char **lend)
{
    const char *s, *e;
    if (it->p >= it->end) {
        return NULL;
    }
    s = it->p;
    e = s;
    while (e < it->end && *e != '\n') {
        e++;
    }
    it->p = (e < it->end) ? e + 1 : e;
    it->line++;
    *lend = e;
    return s;
}

/* Premier mot non blanc de [p,end) ; retourne sa longueur et avance *pp après. */
static size_t next_token(const char **pp, const char *end)
{
    const char *p = *pp;
    const char *s;
    while (p < end && isspace((unsigned char)*p)) {
        p++;
    }
    s = p;
    while (p < end && !isspace((unsigned char)*p)) {
        p++;
    }
    *pp = p;
    return (size_t)(p - s);
}

static int tok_is(const char *p, size_t n, const char *word)
{
    return strlen(word) == n && memcmp(p, word, n) == 0;
}

static void set_err(tamis_verdict *err, long line, long col, const char *msg)
{
    memset(err, 0, sizeof *err);
    err->code = TAMIS_PARSE_ERROR;
    err->sev = TAMIS_SEV_ERROR;
    err->line = line;
    err->column = col;
    err->message = msg;
}

/* Index d'un type par son nom (comparaison bornée par la longueur). */
static int type_index_n(const tamis_model *m, const char *name, size_t len)
{
    size_t i;
    for (i = 0; i < m->n_types; i++) {
        if (strlen(m->type_names[i]) == len && memcmp(m->type_names[i], name, len) == 0) {
            return (int)i;
        }
    }
    return -1;
}

int tamis_model_type_index(const tamis_model *m, const char *name)
{
    return type_index_n(m, name, strlen(name));
}

/* Lit « allow SRC DST [ABILITY] [@RID] » ; p pointe après le mot-clé « allow ». */
static int parse_allow(const char **pp, const char *le, long line, tamis_model *m,
                       tamis_arena *a, tamis_rule *r, tamis_verdict *err)
{
    const char *p = *pp;
    const char *src, *dst, *tok;
    size_t sn, dn, n;
    memset(r, 0, sizeof *r);
    sn = next_token(&p, le); src = p - sn;
    if (sn == 0) { set_err(err, line, 1, "allow : source manquante"); return -1; }
    dn = next_token(&p, le); dst = p - dn;
    if (dn == 0) { set_err(err, line, 1, "allow : cible manquante"); return -1; }
    r->src = type_index_n(m, src, sn);
    r->dst = type_index_n(m, dst, dn);
    if (r->src < 0 || r->dst < 0) {
        set_err(err, line, 1, "allow : type non déclaré");
        return -1;
    }
    r->line = line;
    while ((n = next_token(&p, le)) > 0) {
        tok = p - n;
        if (tok[0] == '@') {
            r->id = tamis_arena_strdup(a, tok + 1, n - 1);
            if (!r->id) { return -2; }
        } else {
            r->ability = tamis_arena_strdup(a, tok, n);
            if (!r->ability) { return -2; }
        }
    }
    *pp = p;
    return 0;
}

/* Lit « untyped NAME ». */
static int parse_untyped(const char **pp, const char *le, long line, tamis_arena *a,
                         tamis_untyped *u, tamis_verdict *err)
{
    const char *p = *pp;
    size_t n = next_token(&p, le);
    if (n == 0) { set_err(err, line, 1, "untyped : nom manquant"); return -1; }
    u->name = tamis_arena_strdup(a, p - n, n);
    if (!u->name) { return -2; }
    u->line = line;
    *pp = p;
    return 0;
}

typedef struct { size_t types, rules, untyped; int defspawn; } counts;

static int count_pass(const char *text, counts *c, tamis_verdict *err)
{
    line_iter it; const char *ls, *le;
    memset(c, 0, sizeof *c);
    li_init(&it, text);
    while ((ls = li_next(&it, &le)) != NULL) {
        const char *p = ls;
        size_t n = next_token(&p, le);
        const char *kw = p - n;
        if (n == 0 || *ls == '#') {
            continue;
        }
        if (tok_is(kw, n, "type")) { c->types++; }
        else if (tok_is(kw, n, "allow")) { c->rules++; }
        else if (tok_is(kw, n, "untyped")) { c->untyped++; }
        else if (tok_is(kw, n, "default_spawn")) { c->defspawn = 1; }
        else {
            set_err(err, it.line, (long)(kw - ls) + 1, "directive inconnue");
            return -1;
        }
    }
    return 0;
}

/* Passe 2 : remplit type_names (tous les `type NAME`). */
static int fill_types(const char *text, tamis_model *m, tamis_arena *a)
{
    line_iter it; const char *ls, *le;
    size_t k = 0;
    li_init(&it, text);
    while ((ls = li_next(&it, &le)) != NULL) {
        const char *p = ls;
        size_t n = next_token(&p, le);
        const char *kw = p - n;
        if (n == 0 || *ls == '#' || !tok_is(kw, n, "type")) {
            continue;
        }
        n = next_token(&p, le);              /* le nom */
        m->type_names[k] = tamis_arena_strdup(a, p - n, n);
        if (!m->type_names[k]) {
            return -2;
        }
        k++;
    }
    return 0;
}

/* Passe 3 : remplit rules + untyped, résout les indices de types. */
static int fill_rules(const char *text, tamis_model *m, tamis_arena *a, tamis_verdict *err)
{
    line_iter it; const char *ls, *le;
    size_t kr = 0, ku = 0;
    li_init(&it, text);
    while ((ls = li_next(&it, &le)) != NULL) {
        const char *p = ls;
        size_t n = next_token(&p, le);
        const char *kw = p - n;
        int rc;
        if (n == 0 || *ls == '#') {
            continue;
        }
        if (tok_is(kw, n, "allow")) {
            rc = parse_allow(&p, le, it.line, m, a, &m->rules[kr++], err);
        } else if (tok_is(kw, n, "untyped")) {
            rc = parse_untyped(&p, le, it.line, a, &m->untyped[ku++], err);
        } else {
            rc = 0;                          /* type / default_spawn : déjà traités */
        }
        if (rc != 0) {
            return rc;
        }
    }
    return 0;
}

int tamis_model_parse(const char *text, tamis_arena *a, tamis_model *out, tamis_verdict *err)
{
    counts c;
    int rc = count_pass(text, &c, err);
    if (rc != 0) {
        return rc;
    }
    memset(out, 0, sizeof *out);
    out->type_names = tamis_arena_alloc(a, c.types * sizeof(char *));
    out->rules = tamis_arena_alloc(a, c.rules * sizeof(tamis_rule));
    out->untyped = tamis_arena_alloc(a, c.untyped * sizeof(tamis_untyped));
    if ((c.types && !out->type_names) || (c.rules && !out->rules) ||
        (c.untyped && !out->untyped)) {
        return -2;
    }
    out->n_types = c.types; out->n_rules = c.rules; out->n_untyped = c.untyped;
    out->has_default_spawn = c.defspawn;
    rc = fill_types(text, out, a);
    if (rc != 0) {
        return rc;
    }
    return fill_rules(text, out, a, err);
}

