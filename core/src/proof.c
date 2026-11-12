/* SPDX-License-Identifier: MIT */
/* Tamis — preuve de non-interférence (TAM-05/20). Réutilise le BFS de reach.c. */
#include "tamis/analyze.h"
#include <stdio.h>
#include <string.h>

static int emit_interference(tamis_out *o, const char *src, const char *dst,
                             const char **path, size_t pl, const char **rules, size_t rl)
{
    tamis_verdict v;
    memset(&v, 0, sizeof v);
    v.code = TAMIS_INTERFERENCE_POSSIBLE;
    v.sev = TAMIS_SEV_ERROR;
    v.src = src; v.dst = dst;
    v.path = path; v.path_len = pl;
    v.rules = rules; v.rules_len = rl;
    v.message = "interférence possible : chemin de privilège trouvé";
    return tamis_out_emit(o, &v);
}

static int emit_certificate(tamis_out *o, const char *src, const char *dst, size_t visits)
{
    tamis_verdict v;
    char *msg = tamis_arena_alloc(o->arena, 256);
    if (!msg) {
        return -2;
    }
    snprintf(msg, 256,
             "non-interférence prouvée : %s n'atteint pas %s — %zu nœud(s) visité(s), "
             "parcours exhaustif", src, dst, visits);
    memset(&v, 0, sizeof v);
    v.code = TAMIS_OK;
    v.sev = TAMIS_SEV_INFO;
    v.src = src; v.dst = dst;
    v.message = msg;
    return tamis_out_emit(o, &v);
}

int tamis_proof(tamis_out *o, const tamis_model *m, const char *src_name, const char *dst_name)
{
    int s = tamis_model_type_index(m, src_name);
    int d = tamis_model_type_index(m, dst_name);
    const char **path = NULL, **rules = NULL;
    size_t pl = 0, rl = 0, visits = 0;
    int rc;
    if (s < 0 || d < 0) {
        return -1;
    }
    rc = tamis_reach_path(m, s, d, o->arena, &path, &pl, &rules, &rl, &visits);
    if (rc < 0) {
        return rc;
    }
    if (rc == 1) {
        emit_interference(o, src_name, dst_name, path, pl, rules, rl);
        return 1;
    }
    emit_certificate(o, src_name, dst_name, visits);
    return 0;
}
