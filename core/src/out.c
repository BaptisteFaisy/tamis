/* SPDX-License-Identifier: MIT */
/* Tamis — contexte d'émission de verdicts partagé par les moteurs (analyze.h). */
#include "tamis/analyze.h"

void tamis_out_init(tamis_out *o, tamis_verdict *v, size_t cap, tamis_arena *a, const char *file)
{
    o->v = v;
    o->cap = cap;
    o->n = 0;
    o->arena = a;
    o->file = file;
}

int tamis_out_emit(tamis_out *o, const tamis_verdict *vd)
{
    if (o->n >= o->cap) {
        return -1;
    }
    o->v[o->n] = *vd;
    if (o->v[o->n].file == NULL) {
        o->v[o->n].file = o->file;
    }
    o->n++;
    return 0;
}
