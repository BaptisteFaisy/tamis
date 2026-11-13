/* SPDX-License-Identifier: MIT */
/* tamis-fuzz — campagne déterministe de fuzz du parseur + moteurs (TAM-11). */
/* Génère des politiques aléatoires par graine (LCG), sans dépendance. À lancer sous ASan/UBSan : */
/* tout crash/erreur mémoire fait avorter le binaire (sortie non nulle). */
#define _POSIX_C_SOURCE 200809L
#include "tamis/analyze.h"
#include "tamis/secpol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long rng;

static unsigned rnd(unsigned n)
{
    rng = rng * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)((rng >> 33) % (n ? n : 1));
}

static char ARENA_BUF[1u << 22];
static tamis_arena A;
static tamis_verdict VS[256];

/* Texte de politique aléatoire : directives valides, inconnues, et parfois du bruit d'octets. */
static void gen_text(char *buf, size_t cap)
{
    static const char *kw[] = {"type", "allow", "untyped", "default_spawn", "garbage", "type"};
    static const char *nm[] = {"a", "b", "c", "d", "e", "f", "g", "h"};
    size_t off = 0;
    int lines = 1 + (int)rnd(12), i, j;
    buf[0] = '\0';
    for (i = 0; i < lines && off + 64 < cap; i++) {
        int w = snprintf(buf + off, cap - off, "%s %s %s %s\n",
                         kw[rnd(6)], nm[rnd(8)], nm[rnd(8)], nm[rnd(8)]);
        if (w < 0) {
            break;
        }
        off += (size_t)w;
    }
    if (rnd(4) == 0 && off + 8 < cap) {
        for (j = 0; j < 6; j++) {
            buf[off++] = (char)(rnd(255) + 1);
        }
        buf[off] = '\0';
    }
}

/* Exerce les moteurs sur un modèle parsé (commun aux deux parseurs). */
static void exercer(tamis_model *m)
{
    tamis_out o;
    tamis_out_init(&o, VS, 256, &A, "fuzz");
    tamis_lint(&o, m);
    if (m->n_types >= 2) {
        tamis_reach(&o, m, m->type_names[0], m->type_names[m->n_types - 1]);
        tamis_proof(&o, m, m->type_names[0], m->type_names[m->n_types - 1]);
        tamis_explain(&o, m, m->type_names[0], m->type_names[m->n_types - 1]);
    }
    tamis_minimize(&o, m);
    tamis_diff(&o, m, m);
}

/* Texte au vocabulaire secpol 8.0 : productions réelles et bruit aléatoire. */
static void gen_secpol(char *buf, size_t cap)
{
    static const char *deb[] = {
        "type", "range", "attribute", "class", "ability", "allow", "allow_attach",
        "allow_link", "derive_type", "default_spawn_type", "mount_point", "garbage"
    };
    static const char *mot[] = {
        "a", "b", "secure", "self", "file", "read", "settypeid", "gain_priv",
        "nonroot", "unlock", "noinherit", "able_create", "setuid", "mem_phys",
        "network/bind", "1024-4096", "0x1200000-", "4", "96-", "/dev/random"
    };
    size_t off = 0;
    int lignes = 1 + (int)rnd(10), i;
    buf[0] = '\0';
    for (i = 0; i < lignes && off + 80 < cap; i++) {
        int w = snprintf(buf + off, cap - off, "%s %s %s %s%s\n",
                         deb[rnd(12)], mot[rnd(20)], mot[rnd(20)], mot[rnd(20)],
                         (rnd(3) == 0) ? ";" : "");
        if (w < 0) { break; }
        off += (size_t)w;
    }
    if (rnd(3) == 0 && off + 8 < cap) {
        int j;
        for (j = 0; j < 5; j++) { buf[off++] = (char)(rnd(255) + 1); }
        buf[off] = '\0';
    }
}

static void run_one(const char *text)
{
    tamis_verdict err;
    tamis_model m;
    tamis_arena_init(&A, ARENA_BUF, sizeof ARENA_BUF);
    if (tamis_model_parse(text, &A, &m, &err) == 0) {
        exercer(&m);              /* format provisoire : accepté */
    }
}

static void run_one_secpol(const char *text)
{
    tamis_verdict err;
    tamis_model m;
    tamis_arena_init(&A, ARENA_BUF, sizeof ARENA_BUF);
    if (tamis_model_parse_secpol(text, &A, &m, &err) == 0) {
        exercer(&m);              /* vrai langage secpol : accepté */
    }
}

int main(int argc, char **argv)
{
    unsigned long seed = (argc > 1) ? strtoul(argv[1], NULL, 10) : 1UL;
    long iters = (argc > 2) ? strtol(argv[2], NULL, 10) : 20000L;
    char text[1024];
    long i;
    rng = seed ? seed : 1UL;
    for (i = 0; i < iters; i++) {
        gen_text(text, sizeof text);
        run_one(text);
        gen_secpol(text, sizeof text);
        run_one_secpol(text);
    }
    printf("OK %ld checks (fuzz graine %lu, 0 crash)\n", iters, seed);
    return 0;
}
