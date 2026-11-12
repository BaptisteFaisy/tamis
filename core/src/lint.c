/* SPDX-License-Identifier: MIT */
/* Tamis — lint moindre privilège (TAM-06) : processus non typés + default_spawn absent. */
#include "tamis/analyze.h"
#include <stdio.h>
#include <string.h>

#define LINT_MSG_SPAWN                                                       \
    "default_spawn_type absent : les processus spawned ne changent pas de type (trou S7)"

/* Émet un verdict WARNING (ligne `line`, message emprunté). 0 ok, -1 tableau plein. */
static int lint_emit(tamis_out *o, tamis_verdict_code code, long line, const char *msg)
{
    tamis_verdict v;
    memset(&v, 0, sizeof v);
    v.code = code;
    v.sev = TAMIS_SEV_WARNING;
    v.line = line;
    v.message = msg;
    return tamis_out_emit(o, &v);
}

/* Formate « processus non typé : <name> » sur l'arène ; NULL si arène épuisée. */
static char *lint_fmt_untyped(tamis_arena *a, const char *name)
{
    static const char PREFIX[] = "processus non typé : ";
    size_t len = sizeof PREFIX + strlen(name);
    char *buf = (char *)tamis_arena_alloc(a, len);
    if (!buf) {
        return NULL;
    }
    snprintf(buf, len, "%s%s", PREFIX, name);
    return buf;
}

int tamis_lint(tamis_out *o, const tamis_model *m)
{
    size_t i;
    int emitted = 0;
    for (i = 0; i < m->n_untyped; i++) {
        char *msg = lint_fmt_untyped(o->arena, m->untyped[i].name);
        if (!msg || lint_emit(o, TAMIS_UNTYPED_PROCESS, m->untyped[i].line, msg) != 0) {
            return -1;
        }
        emitted++;
    }
    if (!m->has_default_spawn &&
        lint_emit(o, TAMIS_MISSING_DEFAULT_SPAWN, 0, LINT_MSG_SPAWN) != 0) {
        return -1;
    }
    emitted += !m->has_default_spawn;
    return emitted;
}
