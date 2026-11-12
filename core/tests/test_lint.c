/* SPDX-License-Identifier: MIT */
/* Tamis — tests de lint.c : moindre privilège (TAM-06). */
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

/* (a) 2 processus non typés (lignes 5 et 6), pas de default_spawn. */
static const char *POL_A =
    "# politique minimale\n"
    "type app_tiers\n"
    "type journal\n"
    "allow app_tiers journal msg @R112\n"
    "untyped nav_ghost\n"
    "untyped spawner\n"
    "type diag\n";

/* (b) default_spawn présent, aucun processus non typé. */
static const char *POL_B =
    "type app_tiers\n"
    "type journal\n"
    "allow app_tiers journal msg @R112\n"
    "default_spawn\n";

/* (c) default_spawn présent mais 2 processus non typés (lignes 3 et 5). */
static const char *POL_C =
    "type app_tiers\n"
    "untyped nav_ghost\n"
    "type journal\n"
    "untyped spawner\n"
    "default_spawn\n";

/* Parse le texte et prépare un contexte de sortie. */
static tamis_out setup(const char *text)
{
    tamis_out o;
    tamis_verdict err;
    tamis_arena_init(&A, ARENA, sizeof ARENA);
    if (tamis_model_parse(text, &A, &M, &err) != 0) {
        printf("FAILED setup : parse du modèle\n");
        g_fail = 1;
    }
    tamis_out_init(&o, VS, 32, &A, "test.mpf");
    return o;
}

/* Cas (a) : 2 untyped + default_spawn absent -> 3 verdicts. */
static void test_cas_a(void)
{
    tamis_out o = setup(POL_A);
    int rc = tamis_lint(&o, &M);
    CHECK(rc == 3, "3 verdicts émis");
    CHECK(o.n == 3, "3 verdicts dans le contexte de sortie");
    CHECK(VS[0].code == TAMIS_UNTYPED_PROCESS, "v0 = UNTYPED_PROCESS");
    CHECK(VS[0].sev == TAMIS_SEV_WARNING, "v0 sévérité warning");
    CHECK(VS[0].line == 5, "v0 ligne 5 (nav_ghost)");
    CHECK(VS[0].message && strcmp(VS[0].message, "processus non typé : nav_ghost") == 0,
          "v0 message exact");
    CHECK(VS[1].code == TAMIS_UNTYPED_PROCESS, "v1 = UNTYPED_PROCESS");
    CHECK(VS[1].sev == TAMIS_SEV_WARNING, "v1 sévérité warning");
    CHECK(VS[1].line == 6, "v1 ligne 6 (spawner)");
    CHECK(VS[1].message && strcmp(VS[1].message, "processus non typé : spawner") == 0,
          "v1 message exact");
    CHECK(VS[2].code == TAMIS_MISSING_DEFAULT_SPAWN, "v2 = MISSING_DEFAULT_SPAWN");
    CHECK(VS[2].sev == TAMIS_SEV_WARNING, "v2 sévérité warning");
    CHECK(VS[2].message && strstr(VS[2].message, "default_spawn_type absent") != NULL,
          "v2 message : default_spawn_type absent");
    CHECK(VS[2].message && strstr(VS[2].message, "trou S7") != NULL,
          "v2 message : trou S7");
    CHECK(VS[0].file && strcmp(VS[0].file, "test.mpf") == 0, "file propagé");
}

/* Cas (b) : default_spawn présent, 0 untyped -> 0 verdict. */
static void test_cas_b(void)
{
    tamis_out o = setup(POL_B);
    int rc = tamis_lint(&o, &M);
    CHECK(rc == 0, "0 verdict émis");
    CHECK(o.n == 0, "contexte de sortie vide");
    CHECK(M.has_default_spawn == 1, "default_spawn détecté");
    CHECK(M.n_untyped == 0, "aucun processus non typé");
}

/* Cas (c) : default_spawn présent, 2 untyped -> 2 verdicts (pas de MISSING). */
static void test_cas_c(void)
{
    tamis_out o = setup(POL_C);
    int rc = tamis_lint(&o, &M);
    CHECK(rc == 2, "2 verdicts émis");
    CHECK(o.n == 2, "2 verdicts dans le contexte de sortie");
    CHECK(VS[0].code == TAMIS_UNTYPED_PROCESS, "v0 = UNTYPED_PROCESS");
    CHECK(VS[0].line == 2, "v0 ligne 2 (nav_ghost)");
    CHECK(VS[1].code == TAMIS_UNTYPED_PROCESS, "v1 = UNTYPED_PROCESS");
    CHECK(VS[1].line == 4, "v1 ligne 4 (spawner)");
}

/* Les messages contiennent bien le nom du processus. */
static void test_messages_containent_noms(void)
{
    tamis_out o = setup(POL_C);
    (void)tamis_lint(&o, &M);
    CHECK(VS[0].message && strstr(VS[0].message, "nav_ghost") != NULL,
          "v0 contient « nav_ghost »");
    CHECK(VS[0].message && strstr(VS[0].message, "processus non typé") != NULL,
          "v0 contient « processus non typé »");
    CHECK(VS[1].message && strstr(VS[1].message, "spawner") != NULL,
          "v1 contient « spawner »");
    CHECK(VS[0].message && strstr(VS[0].message, "spawner") == NULL,
          "v0 ne contient pas « spawner »");
    CHECK(VS[1].message && strstr(VS[1].message, "nav_ghost") == NULL,
          "v1 ne contient pas « nav_ghost »");
}

/* Modèle vide : ni type, ni untyped, ni default_spawn -> 1 verdict. */
static void test_modele_vide(void)
{
    tamis_out o = setup("# rien\n");
    int rc = tamis_lint(&o, &M);
    CHECK(rc == 1, "1 verdict émis");
    CHECK(o.n == 1, "1 verdict dans le contexte de sortie");
    CHECK(VS[0].code == TAMIS_MISSING_DEFAULT_SPAWN, "MISSING_DEFAULT_SPAWN");
    CHECK(VS[0].sev == TAMIS_SEV_WARNING, "sévérité warning");
    CHECK(VS[0].line == 0, "ligne inconnue (0)");
}

/* Arène épuisée : le moteur doit renvoyer une erreur, pas planter. */
static void test_arena_epuisee(void)
{
    tamis_out o = setup(POL_C);
    char petit[16];
    tamis_arena petite_arena;
    tamis_out o2;
    tamis_arena_init(&petite_arena, petit, sizeof petit);
    o2 = o;
    o2.arena = &petite_arena;
    CHECK(tamis_lint(&o2, &M) == -1, "arène épuisée -> -1");
}

/* Tableau de verdicts plein : l'émission échoue, le moteur le signale. */
static void test_sortie_pleine(void)
{
    tamis_out o = setup(POL_C);
    tamis_verdict v1[1];
    tamis_out o2;
    o2 = o;
    o2.v = v1;
    o2.cap = 1;
    o2.n = 0;
    CHECK(tamis_lint(&o2, &M) == -1, "sortie pleine -> -1");
    CHECK(o2.n == 1, "un seul verdict accepté avant saturation");
}

int main(void)
{
    test_cas_a();
    test_cas_b();
    test_cas_c();
    test_messages_containent_noms();
    test_modele_vide();
    test_arena_epuisee();
    test_sortie_pleine();
    if (g_fail) {
        printf("FAILED test_lint\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
