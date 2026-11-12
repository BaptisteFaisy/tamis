/* SPDX-License-Identifier: MIT */
/* Tamis — tests de reach.c : accessibilité transitive + chemin (TAM-04). */
#include "tamis/analyze.h"
#include <stdio.h>
#include <string.h>

static int g_checks = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                    \
    do {                                                                    \
        g_checks++;                                                         \
        if (!(cond)) {                                                      \
            printf("FAILED ligne %d : %s\n", __LINE__, (msg));              \
            g_fail = 1;                                                     \
        }                                                                   \
    } while (0)

static char ARENA[65536];
static tamis_arena A;
static tamis_model M;

/* app_tiers -> journal -> diag -> pompe_critique ; reseau -> journal. */
static const char *POL =
    "type app_tiers\ntype journal\ntype diag\ntype pompe_critique\n"
    "allow app_tiers journal msg @R112\n"
    "allow journal diag msg @R87\n"
    "allow diag pompe_critique control @R9\n"
    "allow reseau journal msg @R10\n";

static tamis_verdict VS[32];

static tamis_out setup(const char *text)
{
    tamis_out o;
    tamis_verdict err;
    tamis_arena_init(&A, ARENA, sizeof ARENA);
    tamis_model_parse(text, &A, &M, &err);
    tamis_out_init(&o, VS, 32, &A, "test.mpf");
    return o;
}

static void test_path_found(void)
{
    tamis_out o = setup("type app_tiers\ntype journal\ntype diag\ntype pompe_critique\n"
                        "allow app_tiers journal msg @R112\n"
                        "allow journal diag msg @R87\n"
                        "allow diag pompe_critique control @R9\n");
    int rc = tamis_reach(&o, &M, "app_tiers", "pompe_critique");
    CHECK(rc == 1, "chemin trouvé");
    CHECK(o.n == 1, "1 verdict émis");
    CHECK(VS[0].code == TAMIS_PATH_FOUND, "code PATH_FOUND");
    CHECK(VS[0].sev == TAMIS_SEV_ERROR, "sévérité error");
    CHECK(VS[0].path_len == 4, "4 nœuds dans le chemin");
    CHECK(strcmp(VS[0].path[0], "app_tiers") == 0, "path[0]=app_tiers");
    CHECK(strcmp(VS[0].path[1], "journal") == 0, "path[1]=journal");
    CHECK(strcmp(VS[0].path[2], "diag") == 0, "path[2]=diag");
    CHECK(strcmp(VS[0].path[3], "pompe_critique") == 0, "path[3]=pompe_critique");
    CHECK(VS[0].rules_len == 3, "3 règles");
    CHECK(strcmp(VS[0].rules[0], "R112") == 0, "rules[0]=R112");
    CHECK(strcmp(VS[0].rules[1], "R87") == 0, "rules[1]=R87");
    CHECK(strcmp(VS[0].rules[2], "R9") == 0, "rules[2]=R9");
    CHECK(strcmp(VS[0].src, "app_tiers") == 0, "src");
    CHECK(strcmp(VS[0].dst, "pompe_critique") == 0, "dst");
    CHECK(VS[0].file && strcmp(VS[0].file, "test.mpf") == 0, "file propagé");
}

static void test_no_path(void)
{
    tamis_out o = setup(POL);
    int rc = tamis_reach(&o, &M, "pompe_critique", "app_tiers");
    CHECK(rc == 0, "aucun chemin (sens inverse)");
    CHECK(o.n == 0, "0 verdict émis");
}

static void test_self(void)
{
    tamis_out o = setup(POL);
    int rc = tamis_reach(&o, &M, "journal", "journal");
    CHECK(rc == 1, "chemin de soi à soi");
    CHECK(VS[0].path_len == 1, "1 seul nœud");
    CHECK(VS[0].rules_len == 0, "0 règle");
}

static void test_unknown_type(void)
{
    tamis_out o = setup(POL);
    CHECK(tamis_reach(&o, &M, "inconnu", "journal") == -1, "type source inconnu -> -1");
    CHECK(tamis_reach(&o, &M, "journal", "inconnu") == -1, "type cible inconnu -> -1");
    CHECK(o.n == 0, "aucun verdict sur erreur");
}

static void test_visits_proof_basis(void)
{
    tamis_out o = setup(POL);
    const char **p = NULL, **r = NULL;
    size_t pl = 0, rl = 0, visits = 0;
    int s = tamis_model_type_index(&M, "pompe_critique");
    int d = tamis_model_type_index(&M, "app_tiers");
    int rc = tamis_reach_path(&M, s, d, &A, &p, &pl, &r, &rl, &visits);
    (void)o;
    CHECK(rc == 0, "reach_path : aucun chemin");
    CHECK(visits == 1, "1 nœud visité (pompe_critique sans successeur) — base de la preuve");
    CHECK(pl == 0 && rl == 0, "chemin vide");
}

static void test_cycle(void)
{
    /* Graphe à cycle : a -> b -> c -> a ; reach a->c doit trouver sans boucler. */
    tamis_out o = setup("type a\ntype b\ntype c\nallow a b\nallow b c\nallow c a\n");
    int rc = tamis_reach(&o, &M, "a", "c");
    CHECK(rc == 1, "chemin trouvé malgré le cycle");
    CHECK(VS[0].path_len == 3, "a->b->c = 3 nœuds");
    CHECK(o.n == 1, "pas de duplication");
}

static void test_invalid_index(void)
{
    tamis_out o = setup(POL);
    const char **p, **r;
    size_t pl, rl, visits;
    (void)o;
    CHECK(tamis_reach_path(&M, -1, 0, &A, &p, &pl, &r, &rl, &visits) == -1, "indice négatif");
    CHECK(tamis_reach_path(&M, 0, 9999, &A, &p, &pl, &r, &rl, &visits) == -1, "indice hors borne");
}

int main(void)
{
    test_path_found();
    test_no_path();
    test_self();
    test_unknown_type();
    test_visits_proof_basis();
    test_cycle();
    test_invalid_index();
    if (g_fail) {
        printf("FAILED test_reach\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
