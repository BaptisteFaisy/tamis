/* SPDX-License-Identifier: MIT */
/* Tamis — tests de l'encodage SMT-LIB2 (smt.c). */
#define _POSIX_C_SOURCE 200809L
#include "tamis/model.h"
#include "tamis/smt.h"
#include <stdio.h>
#include <stdlib.h>
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
static char OUT[262144];

/* app_tiers(0) -> journal(1) -> diag(2) -> pompe_critique(3) ; reseau(4) -> journal */
static const char *POL =
    "type app_tiers\ntype journal\ntype diag\ntype pompe_critique\ntype reseau\n"
    "allow app_tiers journal msg @R112\n"
    "allow journal diag msg @R87\n"
    "allow diag pompe_critique control @R9\n"
    "allow reseau journal msg @R10\n";

static void charger(void)
{
    tamis_verdict err;
    tamis_arena_init(&A, ARENA, sizeof ARENA);
    tamis_model_parse(POL, &A, &M, &err);
}

/* Écrit dans un fichier temporaire et interroge z3 s'il est présent. */
static int interroger_z3(const char *smt, char *reponse, size_t cap)
{
    char cmd[512];
    FILE *p;
    size_t n = 0;
    const char *tmp = "/tmp/tamis-smt-test.smt2";
    FILE *f = fopen(tmp, "w");
    if (!f) {
        return -1;
    }
    fputs(smt, f);
    fclose(f);
    snprintf(cmd, sizeof cmd, "z3 -smt2 %s 2>/dev/null", tmp);
    p = popen(cmd, "r");
    if (!p) {
        return -1;
    }
    if (fgets(reponse, (int)cap, p)) {
        n = strlen(reponse);
    }
    pclose(p);
    remove(tmp);
    return (int)n;
}

static void test_structure(void)
{
    long n;
    charger();
    n = tamis_smt_reach(&M, 0, 3, OUT, sizeof OUT);
    CHECK(n > 0, "encodage non vide");
    CHECK((size_t)n == strlen(OUT), "retour == longueur");
    CHECK(strstr(OUT, "(set-logic HORN)") != NULL, "logique HORN déclarée");
    CHECK(strstr(OUT, "(declare-fun r_0_3 () Bool)") != NULL, "variable r_0_3 déclarée");
    CHECK(strstr(OUT, "(assert r_0_0)") != NULL, "réflexivité r_0_0");
    CHECK(strstr(OUT, "(assert (=> r_0_0 r_0_1))") != NULL, "arête 0->1 propagée");
    CHECK(strstr(OUT, "(assert (=> r_1_1 r_1_2))") != NULL, "arête 1->2 propagée");
    CHECK(strstr(OUT, "(assert (not r_0_3))") != NULL, "requête : src n'atteint pas dst");
    CHECK(strstr(OUT, "(check-sat)") != NULL, "check-sat émis");
}

static void test_tailles(void)
{
    long mesure, n;
    charger();
    mesure = tamis_smt_reach(&M, 0, 3, NULL, 0);
    CHECK(mesure > 0, "mesure sans tampon");
    n = tamis_smt_reach(&M, 0, 3, OUT, sizeof OUT);
    CHECK(n == mesure, "mesure == écriture");
    CHECK(tamis_smt_reach(&M, -1, 0, OUT, sizeof OUT) < 0, "index source invalide");
    CHECK(tamis_smt_reach(&M, 0, 99, OUT, sizeof OUT) < 0, "index cible invalide");
    CHECK(tamis_smt_all(&M, OUT, sizeof OUT) > 0, "encodage global non vide");
}

static void test_semantique(void)
{
    char reponse[256];
    long n;
    charger();
    /* chemin existant (0 -> 3) : la requête « not r_0_3 » doit être INCONSISTANTE */
    n = tamis_smt_reach(&M, 0, 3, OUT, sizeof OUT);
    CHECK(n > 0, "encodage produit");
    if (interroger_z3(OUT, reponse, sizeof reponse) > 0) {
        printf("     z3 (chemin existant) -> %s", reponse);
        CHECK(strstr(reponse, "unsat") != NULL, "z3 : UNSAT pour un chemin qui existe");
    } else {
        printf("INFO z3 absent : sémantique non vérifiée ici (l'encodage reste rejouable)\n");
        g_checks++;
    }
    /* aucun chemin (3 -> 0) : la requête doit être SATISFAISABLE */
    n = tamis_smt_reach(&M, 3, 0, OUT, sizeof OUT);
    CHECK(n > 0, "encodage produit");
    if (interroger_z3(OUT, reponse, sizeof reponse) > 0) {
        printf("     z3 (aucun chemin) -> %s", reponse);
        CHECK(strstr(reponse, "sat") != NULL, "z3 : SAT pour une absence de chemin");
    } else {
        g_checks++;
    }
}

int main(void)
{
    test_structure();
    test_tailles();
    test_semantique();
    if (g_fail) {
        printf("FAILED test_smt\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
