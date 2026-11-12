/* SPDX-License-Identifier: MIT */
/* Tamis — encodage SMT-LIB2 de l'accessibilité (voir smt.h pour la sémantique). */
#include "tamis/smt.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char  *buf;
    size_t cap;
    size_t len;
} w;

static void w_init(w *x, char *buf, size_t cap)
{
    x->buf = buf;
    x->cap = cap;
    x->len = 0;
    if (buf && cap > 0) {
        buf[0] = '\0';
    }
}

static void w_add(w *x, const char *s)
{
    size_t n = strlen(s);
    if (x->buf && x->len + 1 < x->cap) {
        size_t dispo = x->cap - 1 - x->len;
        size_t cp = (n < dispo) ? n : dispo;
        memcpy(x->buf + x->len, s, cp);
        x->buf[x->len + cp] = '\0';
    }
    x->len += n;
}

static void w_fmt(w *x, const char *fmt, ...)
{
    char tmp[512];
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    if (n < 0) {
        return;
    }
    if ((size_t)n < sizeof tmp) {
        w_add(x, tmp);
        return;
    }
    /* plus long que le tampon de ligne : troncature propre (aucune allocation). */
    tmp[sizeof tmp - 1] = '\0';
    w_add(x, tmp);
}

static void emit_header(w *x, const tamis_model *m)
{
    w_add(x, "; Tamis — encodage SMT-LIB2 de l'accessibilité\n");
    w_fmt(x, "; %zu type(s), %zu règle(s) — clauses de Horn (décidable)\n", m->n_types, m->n_rules);
    w_add(x, "; r_i_j = « i atteint j » ; une arête k->j impose r_i_k => r_i_j\n");
    w_add(x, "(set-logic HORN)\n");
}

static void emit_reach_vars(w *x, const tamis_model *m)
{
    size_t i, j;
    for (i = 0; i < m->n_types; i++) {
        for (j = 0; j < m->n_types; j++) {
            w_fmt(x, "(declare-fun r_%zu_%zu () Bool)\n", i, j);
        }
    }
}

static void emit_reflexive(w *x, const tamis_model *m)
{
    size_t i;
    for (i = 0; i < m->n_types; i++) {
        w_fmt(x, "(assert r_%zu_%zu)\n", i, i);
    }
}

static void emit_edges(w *x, const tamis_model *m)
{
    size_t r, i;
    for (r = 0; r < m->n_rules; r++) {
        size_t k = (size_t)m->rules[r].src;
        size_t j = (size_t)m->rules[r].dst;
        for (i = 0; i < m->n_types; i++) {
            w_fmt(x, "(assert (=> r_%zu_%zu r_%zu_%zu))\n", i, k, i, j);
        }
    }
}

static void emit_query(w *x, int src, int dst)
{
    w_fmt(x, "; question : %d atteint-il %d ?\n", src, dst);
    w_add(x, "; UNSAT = le chemin existe (r_src_dst est implique) ; SAT = aucun chemin\n");
    w_fmt(x, "(assert (not r_%d_%d))\n", src, dst);
    w_add(x, "(check-sat)\n");
}

long tamis_smt_reach(const tamis_model *m, int src, int dst, char *buf, size_t cap)
{
    w x;
    if (src < 0 || dst < 0 || (size_t)src >= m->n_types || (size_t)dst >= m->n_types) {
        return -1;
    }
    w_init(&x, buf, cap);
    emit_header(&x, m);
    emit_reach_vars(&x, m);
    emit_reflexive(&x, m);
    emit_edges(&x, m);
    emit_query(&x, src, dst);
    return (long)x.len;
}

long tamis_smt_all(const tamis_model *m, char *buf, size_t cap)
{
    w x;
    size_t i, j;
    w_init(&x, buf, cap);
    emit_header(&x, m);
    emit_reach_vars(&x, m);
    emit_reflexive(&x, m);
    emit_edges(&x, m);
    w_add(&x, "; toutes paires confondues : chaque « (not r_i_j) » doit être SAT\n");
    for (i = 0; i < m->n_types; i++) {
        for (j = 0; j < m->n_types; j++) {
            if (i == j) {
                continue;
            }
            w_fmt(&x, "(push)\n(assert (not r_%zu_%zu))\n(check-sat)\n(pop)\n", i, j);
        }
    }
    return (long)x.len;
}
