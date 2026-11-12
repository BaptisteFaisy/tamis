/* SPDX-License-Identifier: MIT */
/* Tamis — minimisation + preuve d'équivalence (TAM-07). Réutilise tamis_reach_path. */
#include "tamis/analyze.h"
#include <stdio.h>
#include <string.h>

static const char *rule_label(const tamis_rule *r)
{
    if (r->id) { return r->id; }
    if (r->ability) { return r->ability; }
    return "?";
}

/* Matrice d'accessibilité toute-paire (n_types × n_types), allouée sur l'arène. */
static char *all_pairs(const tamis_model *m, tamis_arena *a)
{
    size_t nt = m->n_types, i, j;
    char *mat;
    if (nt == 0) {
        return tamis_arena_alloc(a, 1);
    }
    mat = tamis_arena_alloc(a, nt * nt);
    if (!mat) {
        return NULL;
    }
    memset(mat, 0, nt * nt);
    for (i = 0; i < nt; i++) {
        for (j = 0; j < nt; j++) {
            const char **p, **r;
            size_t pl, rl, vis;
            if (tamis_reach_path(m, (int)i, (int)j, a, &p, &pl, &r, &rl, &vis) == 1) {
                mat[i * nt + j] = 1;
            }
        }
    }
    return mat;
}

/* Atteignabilité src->dst dans un modèle (1 oui, 0 non, <0 erreur). */
static int reach_in(const tamis_model *m, int s, int d, tamis_arena *a)
{
    const char **p, **r;
    size_t pl, rl, vis;
    int rc = tamis_reach_path(m, s, d, a, &p, &pl, &r, &rl, &vis);
    if (rc < 0) {
        return rc;
    }
    return rc == 1;
}

static int emit_redundant(tamis_out *o, const tamis_model *m, const tamis_rule *r)
{
    tamis_verdict v;
    const char **rules = tamis_arena_alloc(o->arena, sizeof(char *));
    char *msg = tamis_arena_alloc(o->arena, 256);
    if (!rules || !msg) {
        return -2;
    }
    rules[0] = rule_label(r);
    snprintf(msg, 256, "règle redondante %s : %s atteint déjà %s sans elle",
             rule_label(r), m->type_names[r->src], m->type_names[r->dst]);
    memset(&v, 0, sizeof v);
    v.code = TAMIS_REDUNDANT_RULE;
    v.sev = TAMIS_SEV_INFO;
    v.line = r->line;
    v.rules = rules; v.rules_len = 1;
    v.message = msg;
    return tamis_out_emit(o, &v);
}

static int emit_equivalence(tamis_out *o, int redundant, size_t pairs, int same)
{
    tamis_verdict v;
    char *msg = tamis_arena_alloc(o->arena, 256);
    if (!msg) {
        return -2;
    }
    snprintf(msg, 256, "minimisation %s : %d règle(s) retirée(s), %zu paire(s) d'accessibilité %s",
             same ? "équivalente" : "NON équivalente (ANOMALIE)", redundant, pairs,
             same ? "identiques" : "DIFFÉRENTES");
    memset(&v, 0, sizeof v);
    v.code = same ? TAMIS_OK : TAMIS_INCONSISTENT;
    v.sev = same ? TAMIS_SEV_INFO : TAMIS_SEV_ERROR;
    v.message = msg;
    return tamis_out_emit(o, &v);
}

int tamis_minimize(tamis_out *o, const tamis_model *m)
{
    size_t i, k = 0, nt = m->n_types;
    tamis_rule *kept;
    tamis_model minm;
    char *before, *after;
    int emitted = 0, redundant = 0, same;
    kept = tamis_arena_alloc(o->arena, (m->n_rules ? m->n_rules : 1) * sizeof(tamis_rule));
    if (!kept) {
        return -2;
    }
    minm = *m;
    minm.rules = kept;
    /* Retrait glouton : une règle est redondante si src atteint déjà dst via les règles
       conservées. Comme on ne retire que des arêtes déjà implicites, l'accessibilité
       toute-paire est préservée (vérifié ci-dessous). */
    for (i = 0; i < m->n_rules; i++) {
        int red;
        minm.n_rules = k;
        red = reach_in(&minm, m->rules[i].src, m->rules[i].dst, o->arena);
        if (red < 0) {
            return red;
        }
        if (red) {
            if (emit_redundant(o, m, &m->rules[i]) != 0) {
                return -1;
            }
            emitted++;
            redundant++;
        } else {
            kept[k++] = m->rules[i];
        }
    }
    minm.n_rules = k;
    before = all_pairs(m, o->arena);
    after = all_pairs(&minm, o->arena);
    if (!before || !after) {
        return -2;
    }
    same = (nt == 0) || (memcmp(before, after, nt * nt) == 0);
    if (emit_equivalence(o, redundant, nt * nt, same) != 0) {
        return -1;
    }
    return emitted + 1;
}
