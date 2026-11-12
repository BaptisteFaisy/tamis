/* SPDX-License-Identifier: MIT */
/* Tamis — tests de diff.c : élargissements/réductions de privilège (TAM-08). */
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

static char AR1[65536], AR2[65536];
static tamis_arena A1, A2;
static tamis_model M1, M2;
static tamis_verdict VS[32];
static tamis_out O;

static void setup2(const char *base, const char *cand)
{
    tamis_verdict err;
    tamis_arena_init(&A1, AR1, sizeof AR1);
    tamis_arena_init(&A2, AR2, sizeof AR2);
    tamis_model_parse(base, &A1, &M1, &err);
    tamis_model_parse(cand, &A2, &M2, &err);
    tamis_out_init(&O, VS, 32, &A1, "diff");
}

static int count_code(tamis_verdict_code c)
{
    int n = 0;
    size_t i;
    for (i = 0; i < O.n; i++) {
        if (VS[i].code == c) {
            n++;
        }
    }
    return n;
}

static void test_narrowed(void)
{
    int rc = (setup2("type a\ntype b\ntype c\nallow a b\nallow b c\n",
                     "type a\ntype b\ntype c\nallow a b\n"),
              tamis_diff(&O, &M1, &M2));
    CHECK(rc == 1, "1 changement");
    CHECK(count_code(TAMIS_PRIVILEGE_NARROWED) == 1, "1 réduction");
    CHECK(count_code(TAMIS_PRIVILEGE_WIDENED) == 0, "0 élargissement");
    CHECK(VS[0].sev == TAMIS_SEV_INFO, "réduction = info");
    CHECK(VS[0].src && strcmp(VS[0].src, "b") == 0, "src=b");
    CHECK(VS[0].dst && strcmp(VS[0].dst, "c") == 0, "dst=c");
}

static void test_widened(void)
{
    int rc = (setup2("type a\ntype b\ntype c\nallow a b\n",
                     "type a\ntype b\ntype c\nallow a b\nallow b c\n"),
              tamis_diff(&O, &M1, &M2));
    CHECK(rc == 1, "1 changement");
    CHECK(count_code(TAMIS_PRIVILEGE_WIDENED) == 1, "1 élargissement");
    CHECK(count_code(TAMIS_PRIVILEGE_NARROWED) == 0, "0 réduction");
    CHECK(VS[0].sev == TAMIS_SEV_ERROR, "élargissement = error");
    CHECK(VS[0].message && strstr(VS[0].message, "élargissement") != NULL, "message");
}

static void test_identical(void)
{
    const char *p = "type a\ntype b\nallow a b msg\n";
    int rc = (setup2(p, p), tamis_diff(&O, &M1, &M2));
    CHECK(rc == 0, "politiques identiques -> 0 verdict");
    CHECK(O.n == 0, "aucun verdict");
}

static void test_ability_change(void)
{
    int rc = (setup2("type a\ntype b\nallow a b msg\n",
                     "type a\ntype b\nallow a b control\n"),
              tamis_diff(&O, &M1, &M2));
    CHECK(rc == 2, "changement d'ability = retrait + ajout");
    CHECK(count_code(TAMIS_PRIVILEGE_WIDENED) == 1, "1 élargissement (control)");
    CHECK(count_code(TAMIS_PRIVILEGE_NARROWED) == 1, "1 réduction (msg)");
}

int main(void)
{
    test_narrowed();
    test_widened();
    test_identical();
    test_ability_change();
    if (g_fail) {
        printf("FAILED test_diff\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
