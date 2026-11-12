/* SPDX-License-Identifier: MIT */
/* Tamis — tests de model.c (construction du graphe depuis le format provisoire). */
#include "tamis/model.h"
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

static int parse(const char *text, tamis_verdict *err)
{
    tamis_arena_init(&A, ARENA, sizeof ARENA);
    return tamis_model_parse(text, &A, &M, err);
}

static void test_parse_ok(void)
{
    const char *txt = "type a\ntype b\nallow a b msg @R1\nuntyped /bin/x\ndefault_spawn\n";
    tamis_verdict err;
    CHECK(parse(txt, &err) == 0, "parse ok");
    CHECK(M.n_types == 2, "2 types");
    CHECK(M.n_rules == 1, "1 règle");
    CHECK(M.n_untyped == 1, "1 untyped");
    CHECK(M.has_default_spawn == 1, "default_spawn présent");
    CHECK(strcmp(M.type_names[0], "a") == 0, "type_names[0]==a");
    CHECK(strcmp(M.type_names[1], "b") == 0, "type_names[1]==b");
    CHECK(M.rules[0].src == 0 && M.rules[0].dst == 1, "arête a->b");
    CHECK(M.rules[0].ability && strcmp(M.rules[0].ability, "msg") == 0, "ability msg");
    CHECK(M.rules[0].id && strcmp(M.rules[0].id, "R1") == 0, "id R1");
    CHECK(M.rules[0].line == 3, "règle ligne 3");
    CHECK(strcmp(M.untyped[0].name, "/bin/x") == 0, "untyped /bin/x");
    CHECK(M.untyped[0].line == 4, "untyped ligne 4");
    CHECK(tamis_model_type_index(&M, "a") == 0, "index de a");
    CHECK(tamis_model_type_index(&M, "b") == 1, "index de b");
    CHECK(tamis_model_type_index(&M, "z") == -1, "index de z absent");
}

static void test_optional_fields(void)
{
    const char *txt = "type a\ntype b\nallow a b\n";
    tamis_verdict err;
    CHECK(parse(txt, &err) == 0, "allow sans ability/id ok");
    CHECK(M.rules[0].ability == NULL, "ability NULL");
    CHECK(M.rules[0].id == NULL, "id NULL");
    CHECK(M.has_default_spawn == 0, "pas de default_spawn");
    CHECK(M.n_untyped == 0, "pas d'untyped");
}

static void test_comments_and_blank(void)
{
    const char *txt = "# commentaire\n\n   \ntype a\n# autre\ntype b\n";
    tamis_verdict err;
    CHECK(parse(txt, &err) == 0, "commentaires/lignes vides ignorés");
    CHECK(M.n_types == 2, "2 types malgré commentaires");
}

static void test_parse_errors(void)
{
    tamis_verdict err;
    CHECK(parse("type a\nfoo bar\n", &err) == -1, "directive inconnue -> -1");
    CHECK(err.code == TAMIS_PARSE_ERROR, "code PARSE_ERROR");
    CHECK(err.line == 2, "erreur ligne 2");
    CHECK(err.sev == TAMIS_SEV_ERROR, "sévérité error");
    CHECK(parse("allow a b\n", &err) == -1, "type non déclaré -> -1");
    CHECK(err.code == TAMIS_PARSE_ERROR, "code PARSE_ERROR (type non déclaré)");
    CHECK(parse("type a\nallow a\n", &err) == -1, "allow cible manquante -> -1");
    CHECK(parse("untyped\n", &err) == -1, "untyped nom manquant -> -1");
    CHECK(parse("allow\n", &err) == -1, "allow source manquante -> -1");
}

static void test_empty(void)
{
    tamis_verdict err;
    CHECK(parse("", &err) == 0, "texte vide accepté");
    CHECK(M.n_types == 0 && M.n_rules == 0 && M.n_untyped == 0, "modèle vide");
}

static void test_arena_exhausted(void)
{
    tamis_arena small;
    char buf[8];
    tamis_model m;
    tamis_verdict err;
    tamis_arena_init(&small, buf, sizeof buf);
    CHECK(tamis_model_parse("type a\ntype b\nallow a b\n", &small, &m, &err) == -2,
          "arène trop petite -> -2");
}

static void test_arena_alloc(void)
{
    tamis_arena a;
    char buf[64];
    char *p1, *p2;
    tamis_arena_init(&a, buf, sizeof buf);
    p1 = tamis_arena_alloc(&a, 8);
    p2 = tamis_arena_alloc(&a, 8);
    CHECK(p1 != NULL && p2 != NULL, "allocations ok");
    CHECK(p2 > p1, "bump vers l'avant");
    CHECK(((size_t)(p1 - buf) % 8) == 0, "alignement 8");
    CHECK(tamis_arena_alloc(&a, 1000) == NULL, "allocation trop grande -> NULL");
    CHECK(a.used <= sizeof buf, "used borné");
}

int main(void)
{
    test_parse_ok();
    test_optional_fields();
    test_comments_and_blank();
    test_parse_errors();
    test_empty();
    test_arena_exhausted();
    test_arena_alloc();
    if (g_fail) {
        printf("FAILED test_model\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
