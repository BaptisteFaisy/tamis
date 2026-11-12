/* SPDX-License-Identifier: MIT */
/* Tamis — diff sémantique entre deux politiques (TAM-08). Compare par NOMS de types. */
#include "tamis/analyze.h"
#include <stdio.h>
#include <string.h>

static const char *rule_ability(const tamis_rule *r)
{
    return r->ability ? r->ability : "";
}

/* Vrai si m contient une arête src->dst avec cette ability (comparaison par noms). */
static int has_edge(const tamis_model *m, const char *src, const char *dst, const char *ability)
{
    size_t i;
    for (i = 0; i < m->n_rules; i++) {
        const tamis_rule *r = &m->rules[i];
        if (strcmp(m->type_names[r->src], src) == 0 &&
            strcmp(m->type_names[r->dst], dst) == 0 &&
            strcmp(rule_ability(r), ability) == 0) {
            return 1;
        }
    }
    return 0;
}

static int emit_edge_change(tamis_out *o, const tamis_model *from, const tamis_rule *r, int widened)
{
    tamis_verdict v;
    const char *src = from->type_names[r->src];
    const char *dst = from->type_names[r->dst];
    char *msg = tamis_arena_alloc(o->arena, 256);
    if (!msg) {
        return -2;
    }
    snprintf(msg, 256, "%s : %s -> %s [%s]",
             widened ? "élargissement de privilège" : "réduction de privilège",
             src, dst, rule_ability(r));
    memset(&v, 0, sizeof v);
    v.code = widened ? TAMIS_PRIVILEGE_WIDENED : TAMIS_PRIVILEGE_NARROWED;
    v.sev = widened ? TAMIS_SEV_ERROR : TAMIS_SEV_INFO;
    v.src = src; v.dst = dst;
    v.message = msg;
    return tamis_out_emit(o, &v);
}

int tamis_diff(tamis_out *o, const tamis_model *base, const tamis_model *cand)
{
    size_t i;
    int emitted = 0;
    for (i = 0; i < cand->n_rules; i++) {
        const tamis_rule *r = &cand->rules[i];
        if (!has_edge(base, cand->type_names[r->src], cand->type_names[r->dst], rule_ability(r))) {
            if (emit_edge_change(o, cand, r, 1) != 0) { return -1; }
            emitted++;
        }
    }
    for (i = 0; i < base->n_rules; i++) {
        const tamis_rule *r = &base->rules[i];
        if (!has_edge(cand, base->type_names[r->src], base->type_names[r->dst], rule_ability(r))) {
            if (emit_edge_change(o, base, r, 0) != 0) { return -1; }
            emitted++;
        }
    }
    return emitted;
}
