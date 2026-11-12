/* SPDX-License-Identifier: MIT */
/* Tamis — tests de minimize.c : règles redondantes + preuve d'équivalence (TAM-07). */
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
static tamis_out O;

static void setup(const char *text)
{
    tamis_verdict err;
    tamis_arena_init(&A, ARENA, sizeof ARENA);
    tamis_model_parse(text, &A, &M, &err);
    tamis_out_init(&O, VS, 32, &A, "test.mpf");
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

static const tamis_verdict *find_equiv(void)
{
    size_t i;
    for (i = 0; i < O.n; i++) {
        if (VS[i].code == TAMIS_OK || VS[i].code == TAMIS_INCONSISTENT) {
            return &VS[i];
        }
    }
    return NULL;
}

static void test_duplicate(void)
{
    const tamis_verdict *eq;
    int rc;
    setup("type a\ntype b\nallow a b msg @R1\nallow a b msg @R2\n");
    rc = tamis_minimize(&O, &M);
    CHECK(rc == 2, "1 redondante + 1 équivalence");
    CHECK(count_code(TAMIS_REDUNDANT_RULE) == 1, "1 règle redondante");
    CHECK(VS[0].rules_len == 1 && strcmp(VS[0].rules[0], "R2") == 0, "R2 marquée redondante");
    CHECK(VS[0].line == 4, "ligne de R2");
    eq = find_equiv();
    CHECK(eq != NULL && eq->code == TAMIS_OK, "équivalence = OK");
    CHECK(eq && strstr(eq->message, "équivalente") != NULL, "message 'équivalente'");
    CHECK(eq && strstr(eq->message, "identiques") != NULL, "paires identiques");
    CHECK(eq && strstr(eq->message, "1 règle") != NULL, "1 règle retirée");
}

static void test_no_redundant(void)
{
    const tamis_verdict *eq;
    int rc;
    setup("type a\ntype b\ntype c\nallow a b\nallow b c\n");
    rc = tamis_minimize(&O, &M);
    CHECK(rc == 1, "seule l'équivalence");
    CHECK(count_code(TAMIS_REDUNDANT_RULE) == 0, "0 redondante");
    eq = find_equiv();
    CHECK(eq != NULL && eq->code == TAMIS_OK, "équivalence OK");
    CHECK(eq && strstr(eq->message, "0 règle") != NULL, "0 règle retirée");
}

static void test_transitive(void)
{
    const tamis_verdict *eq;
    setup("type a\ntype b\ntype c\nallow a b\nallow b c\nallow a c\n");
    tamis_minimize(&O, &M);
    CHECK(count_code(TAMIS_REDUNDANT_RULE) == 1, "a->c redondante (transitive)");
    CHECK(VS[0].rules_len == 1 && strcmp(VS[0].rules[0], "?") == 0, "label '?' (pas d'id/ability)");
    eq = find_equiv();
    CHECK(eq && eq->code == TAMIS_OK, "équivalence préservée malgré le retrait");
    CHECK(eq && strstr(eq->message, "identiques") != NULL, "accessibilité inchangée");
}

static void test_chain_kept(void)
{
    /* Chaîne sans doublon : chaque règle est nécessaire, aucune retirée. */
    setup("type a\ntype b\ntype c\ntype d\nallow a b\nallow b c\nallow c d\n");
    tamis_minimize(&O, &M);
    CHECK(count_code(TAMIS_REDUNDANT_RULE) == 0, "chaîne : 0 redondante");
    CHECK(find_equiv()->code == TAMIS_OK, "équivalence OK");
}

int main(void)
{
    test_duplicate();
    test_no_redundant();
    test_transitive();
    test_chain_kept();
    if (g_fail) {
        printf("FAILED test_minimize\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
