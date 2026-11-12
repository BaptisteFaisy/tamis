/* SPDX-License-Identifier: MIT */
/* Tamis — tests de proof.c : certificat de non-interférence (TAM-05/20). */
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

static void test_interference(void)
{
    tamis_out o = setup(POL);
    int rc = tamis_proof(&o, &M, "app_tiers", "pompe_critique");
    CHECK(rc == 1, "interférence détectée -> 1");
    CHECK(o.n == 1, "1 verdict");
    CHECK(VS[0].code == TAMIS_INTERFERENCE_POSSIBLE, "code INTERFERENCE_POSSIBLE");
    CHECK(VS[0].sev == TAMIS_SEV_ERROR, "sévérité error");
    CHECK(VS[0].path_len == 4, "chemin de 4 nœuds");
    CHECK(VS[0].rules_len == 3, "3 règles");
    CHECK(strcmp(VS[0].src, "app_tiers") == 0, "src");
    CHECK(strcmp(VS[0].dst, "pompe_critique") == 0, "dst");
}

static void test_certificate(void)
{
    tamis_out o = setup(POL);
    int rc = tamis_proof(&o, &M, "pompe_critique", "app_tiers");
    CHECK(rc == 0, "absence prouvée -> 0");
    CHECK(o.n == 1, "1 certificat émis");
    CHECK(VS[0].code == TAMIS_OK, "code TAMIS_OK (certificat)");
    CHECK(VS[0].sev == TAMIS_SEV_INFO, "sévérité info");
    CHECK(VS[0].message && strstr(VS[0].message, "prouvée") != NULL, "message contient 'prouvée'");
    CHECK(VS[0].message && strstr(VS[0].message, "visit") != NULL, "message contient le compteur de visites");
    CHECK(VS[0].message && strstr(VS[0].message, "exhaustif") != NULL, "message mentionne le parcours exhaustif");
}

static void test_unknown(void)
{
    tamis_out o = setup(POL);
    CHECK(tamis_proof(&o, &M, "fantome", "journal") == -1, "type source inconnu -> -1");
    CHECK(tamis_proof(&o, &M, "journal", "fantome") == -1, "type cible inconnu -> -1");
    CHECK(o.n == 0, "aucun verdict sur erreur");
}

static void test_cut_path(void)
{
    /* Politique durcie : R9 supprimée, plus de chemin -> certificat. */
    tamis_out o = setup("type app_tiers\ntype journal\ntype diag\ntype pompe_critique\n"
                        "allow app_tiers journal msg @R112\n"
                        "allow journal diag msg @R87\n");
    int rc = tamis_proof(&o, &M, "app_tiers", "pompe_critique");
    CHECK(rc == 0, "chemin coupé -> absence prouvée");
    CHECK(VS[0].code == TAMIS_OK, "certificat vert après correctif");
}

int main(void)
{
    test_interference();
    test_certificate();
    test_unknown();
    test_cut_path();
    if (g_fail) {
        printf("FAILED test_proof\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
