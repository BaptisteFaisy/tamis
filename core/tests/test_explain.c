/* SPDX-License-Identifier: MIT */
/* Tamis — tests de explain.c : explication pédagogique + correctif + angles morts. */
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
static tamis_verdict VS[32];

static const char *POL =
    "type app_tiers\ntype journal\ntype diag\ntype pompe_critique\n"
    "allow app_tiers journal msg @R112\n"
    "allow journal diag msg @R87\n"
    "allow diag pompe_critique control @R9\n";

static tamis_out setup(const char *text)
{
    tamis_out o;
    tamis_verdict err;
    tamis_arena_init(&A, ARENA, sizeof ARENA);
    tamis_model_parse(text, &A, &M, &err);
    tamis_out_init(&o, VS, 32, &A, "test.mpf");
    return o;
}

static void test_path_explained(void)
{
    tamis_out o = setup(POL);
    int rc = tamis_explain(&o, &M, "app_tiers", "pompe_critique");
    CHECK(rc == 1, "chemin trouvé -> 1");
    CHECK(o.n == 2, "PATH_FOUND + explication");
    CHECK(VS[0].code == TAMIS_PATH_FOUND, "d'abord le verdict de chemin");
    CHECK(VS[1].code == TAMIS_OK && VS[1].sev == TAMIS_SEV_INFO, "puis l'explication (info)");
    CHECK(strstr(VS[1].message, "Pourquoi") != NULL, "explication 'Pourquoi'");
    CHECK(strstr(VS[1].message, "Correctif minimal") != NULL, "correctif minimal proposé");
    CHECK(strstr(VS[1].message, "R9") != NULL, "nomme la règle à couper (R9, dernière arête)");
    CHECK(strstr(VS[1].message, "app_tiers →(R112) journal") != NULL, "chaîne lisible");
    CHECK(strstr(VS[1].message, "pompe_critique") != NULL, "chaîne jusqu'à la cible");
}

static void test_safe_explained(void)
{
    tamis_out o = setup(POL);
    int rc = tamis_explain(&o, &M, "pompe_critique", "app_tiers");
    CHECK(rc == 0, "aucun chemin -> 0");
    CHECK(o.n == 1, "une seule explication (pas de PATH_FOUND)");
    CHECK(VS[0].code == TAMIS_OK, "verdict sain");
    CHECK(strstr(VS[0].message, "non-interférence prouvée") != NULL, "non-interférence");
    CHECK(strstr(VS[0].message, "Périmètre de complétude") != NULL, "périmètre publié");
    CHECK(strstr(VS[0].message, "Angles morts") != NULL, "angles morts déclarés");
    CHECK(strstr(VS[0].message, "TAM-21") != NULL, "référence TAM-21");
}

static void test_unknown(void)
{
    tamis_out o = setup(POL);
    CHECK(tamis_explain(&o, &M, "fantome", "journal") == -1, "type inconnu -> -1");
    CHECK(o.n == 0, "aucun verdict sur erreur");
}

int main(void)
{
    test_path_explained();
    test_safe_explained();
    test_unknown();
    if (g_fail) {
        printf("FAILED test_explain\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
