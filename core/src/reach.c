/* SPDX-License-Identifier: MIT */
/* Tamis — accessibilité transitive (TAM-04) + BFS partagé avec la preuve (TAM-05). */
#include "tamis/analyze.h"
#include <string.h>

/* Libellé d'une règle : son id, sinon son ability, sinon "?". */
static const char *rule_label(const tamis_rule *r)
{
    if (r->id) {
        return r->id;
    }
    if (r->ability) {
        return r->ability;
    }
    return "?";
}

/* Reconstruit le chemin s->d depuis les tableaux parent/prule (BFS). */
static int build_path(const tamis_model *m, int s, int d, const int *parent,
                      const int *prule, tamis_arena *a, const char ***path,
                      size_t *path_len, const char ***rules, size_t *rules_len)
{
    size_t len = 0, i;
    int cur = d;
    const char **pn, **rn;
    while (cur != -1) {
        len++;
        if (cur == s) {
            break;
        }
        cur = parent[cur];
    }
    pn = tamis_arena_alloc(a, len * sizeof(char *));
    rn = (len > 1) ? tamis_arena_alloc(a, (len - 1) * sizeof(char *)) : NULL;
    if (!pn || (len > 1 && !rn)) {
        return -2;
    }
    i = len;
    cur = d;
    while (cur != -1) {
        i--;
        pn[i] = m->type_names[cur];
        if (i > 0) {
            rn[i - 1] = rule_label(&m->rules[prule[cur]]);
        }
        if (cur == s) {
            break;
        }
        cur = parent[cur];
    }
    *path = pn; *path_len = len;
    *rules = rn; *rules_len = (len > 1) ? len - 1 : 0;
    return 1;
}

int tamis_reach_path(const tamis_model *m, int s, int d, tamis_arena *a,
                     const char ***path, size_t *path_len,
                     const char ***rules, size_t *rules_len, size_t *visits)
{
    size_t nt = m->n_types, qh = 0, qt = 0, nvis = 0, r;
    char *visited;
    int *parent, *prule, *queue, found;
    if (s < 0 || d < 0 || (size_t)s >= nt || (size_t)d >= nt) {
        return -1;
    }
    visited = tamis_arena_alloc(a, nt ? nt : 1);
    parent = tamis_arena_alloc(a, (nt ? nt : 1) * sizeof(int));
    prule = tamis_arena_alloc(a, (nt ? nt : 1) * sizeof(int));
    queue = tamis_arena_alloc(a, (nt ? nt : 1) * sizeof(int));
    if (!visited || !parent || !prule || !queue) {
        return -2;
    }
    memset(visited, 0, nt);
    visited[s] = 1; parent[s] = -1; prule[s] = -1; queue[qt++] = s; nvis = 1;
    found = (s == d);
    while (qh < qt && !found) {
        int u = queue[qh++];
        for (r = 0; r < m->n_rules; r++) {
            int v;
            if (m->rules[r].src != u) {
                continue;
            }
            v = m->rules[r].dst;
            if (visited[v]) {
                continue;
            }
            visited[v] = 1; parent[v] = u; prule[v] = (int)r; nvis++;
            if (v == d) {
                found = 1;
                break;
            }
            queue[qt++] = v;
        }
    }
    *visits = nvis;
    if (!found) {
        *path = NULL; *path_len = 0; *rules = NULL; *rules_len = 0;
        return 0;
    }
    return build_path(m, s, d, parent, prule, a, path, path_len, rules, rules_len);
}

int tamis_reach(tamis_out *o, const tamis_model *m, const char *src_name, const char *dst_name)
{
    int s = tamis_model_type_index(m, src_name);
    int d = tamis_model_type_index(m, dst_name);
    const char **path = NULL, **rules = NULL;
    size_t pl = 0, rl = 0, visits = 0;
    int rc;
    tamis_verdict v;
    if (s < 0 || d < 0) {
        return -1;
    }
    rc = tamis_reach_path(m, s, d, o->arena, &path, &pl, &rules, &rl, &visits);
    if (rc <= 0) {
        return rc;
    }
    memset(&v, 0, sizeof v);
    v.code = TAMIS_PATH_FOUND;
    v.sev = TAMIS_SEV_ERROR;
    v.src = src_name; v.dst = dst_name;
    v.path = path; v.path_len = pl;
    v.rules = rules; v.rules_len = rl;
    v.message = "chemin de privilège atteignable";
    tamis_out_emit(o, &v);
    return 1;
}
