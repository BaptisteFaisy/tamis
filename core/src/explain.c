/* SPDX-License-Identifier: MIT */
/* Tamis — explication pédagogique d'un verdict + correctif minimal + angles morts. */
/* Idées 26/51/65 (Palier 1) : rendre l'outil utilisable par quelqu'un d'autre que l'auteur. */
#include "tamis/analyze.h"
#include <stdio.h>
#include <string.h>

#define MSG_CAP 1024

/* Chaîne lisible du chemin : n0 →(R0) n1 →(R1) n2 … (allouée sur l'arène). */
static const char *build_chain(tamis_arena *a, const char *const *path, size_t pl,
                               const char *const *rules, size_t rl)
{
    char *buf = tamis_arena_alloc(a, MSG_CAP);
    size_t off;
    size_t i;
    if (!buf || pl == 0) {
        return NULL;
    }
    off = (size_t)snprintf(buf, MSG_CAP, "%s", path[0]);
    for (i = 0; i < rl && i + 1 < pl && off < MSG_CAP; i++) {
        int k = snprintf(buf + off, MSG_CAP - off, " →(%s) %s", rules[i], path[i + 1]);
        if (k < 0) {
            break;
        }
        off += (size_t)k;
    }
    return buf;
}

static int emit_explain(tamis_out *o, const char *src, const char *dst, const char *msg)
{
    tamis_verdict v;
    memset(&v, 0, sizeof v);
    v.code = TAMIS_OK;
    v.sev = TAMIS_SEV_INFO;
    v.src = src;
    v.dst = dst;
    v.message = msg;
    return tamis_out_emit(o, &v);
}

/* Explication d'un chemin trouvé : pourquoi + correctif minimal (la dernière arête). */
static int explain_path(tamis_out *o, const tamis_verdict *pv, const char *dst)
{
    char *msg = tamis_arena_alloc(o->arena, MSG_CAP);
    const char *chain = build_chain(o->arena, pv->path, pv->path_len, pv->rules, pv->rules_len);
    const char *last = (pv->rules_len > 0) ? pv->rules[pv->rules_len - 1] : "la dernière règle";
    if (!msg || !chain) {
        return -2;
    }
    snprintf(msg, MSG_CAP,
             "Pourquoi ce refus : %s atteint %s via la chaîne %s. "
             "Correctif minimal : retirer la règle %s (dernière arête avant %s) tue ce chemin ; "
             "relancer `tamis proof --src %s --dst %s` pour obtenir le certificat de non-interférence.",
             pv->src, dst, chain, last, dst, pv->src, dst);
    emit_explain(o, pv->src, dst, msg);
    return 1;
}

/* Aucune interception : publie le périmètre de complétude et les angles morts bornés. */
static int explain_safe(tamis_out *o, const tamis_model *m, const char *src, const char *dst)
{
    char *msg = tamis_arena_alloc(o->arena, MSG_CAP);
    const char **p, **r;
    size_t pl, rl, vis = 0;
    int s = tamis_model_type_index(m, src), d = tamis_model_type_index(m, dst);
    if (!msg) {
        return -2;
    }
    if (s >= 0 && d >= 0) {
        tamis_reach_path(m, s, d, o->arena, &p, &pl, &r, &rl, &vis);
    }
    snprintf(msg, MSG_CAP,
             "Aucun chemin %s → %s : non-interférence prouvée (%zu nœud(s) visité(s), parcours exhaustif). "
             "Périmètre de complétude : flux explicites de type à type du modèle. "
             "Angles morts bornés (TAM-21) : flux IPC indirects (MsgSend/ConnectAttach), "
             "permissions de fichiers et transitions root→non-root — NON couverts, à traiter hors Tamis.",
             src, dst, vis);
    emit_explain(o, src, dst, msg);
    return 0;
}

int tamis_explain(tamis_out *o, const tamis_model *m, const char *src_name, const char *dst_name)
{
    int rc = tamis_reach(o, m, src_name, dst_name);
    if (rc < 0) {
        return rc;
    }
    if (rc == 1) {
        return explain_path(o, &o->v[o->n - 1], dst_name);
    }
    return explain_safe(o, m, src_name, dst_name);
}
