/* SPDX-License-Identifier: MIT */
/* Tamis — tests unitaires de verdict.c (CONTRACT.md §3, §4). Binaire autonome. */
#include "tamis/verdict.h"
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

static const char *const EXPECT_NAMES[TAMIS_VERDICT__COUNT] = {
    "TAMIS_OK", "TAMIS_PARSE_ERROR", "TAMIS_UNTYPED_PROCESS",
    "TAMIS_MISSING_DEFAULT_SPAWN", "TAMIS_DEAD_RULE", "TAMIS_REDUNDANT_RULE",
    "TAMIS_OVERBROAD_ABILITY", "TAMIS_RANGE_INVALID", "TAMIS_PATH_FOUND",
    "TAMIS_INTERFERENCE_POSSIBLE", "TAMIS_PRIVILEGE_WIDENED",
    "TAMIS_PRIVILEGE_NARROWED", "TAMIS_INCONSISTENT",
};

static const tamis_severity EXPECT_SEV[TAMIS_VERDICT__COUNT] = {
    TAMIS_SEV_INFO, TAMIS_SEV_ERROR, TAMIS_SEV_WARNING, TAMIS_SEV_WARNING,
    TAMIS_SEV_INFO, TAMIS_SEV_INFO, TAMIS_SEV_WARNING, TAMIS_SEV_ERROR,
    TAMIS_SEV_ERROR, TAMIS_SEV_ERROR, TAMIS_SEV_ERROR, TAMIS_SEV_INFO,
    TAMIS_SEV_ERROR,
};

static void test_names(void)
{
    int i;
    for (i = 0; i < (int)TAMIS_VERDICT__COUNT; i++) {
        const char *n = tamis_verdict_name((tamis_verdict_code)i);
        CHECK(n != NULL, "nom non NULL");
        CHECK(strcmp(n, EXPECT_NAMES[i]) == 0, "nom canonique exact");
        CHECK(strncmp(n, "TAMIS_", 6) == 0, "préfixe TAMIS_");
        CHECK(tamis_verdict_code_valid((tamis_verdict_code)i) == 1, "code valide");
    }
}

static void test_severities(void)
{
    int i;
    CHECK(strcmp(tamis_severity_name(TAMIS_SEV_INFO), "info") == 0, "info");
    CHECK(strcmp(tamis_severity_name(TAMIS_SEV_WARNING), "warning") == 0, "warning");
    CHECK(strcmp(tamis_severity_name(TAMIS_SEV_ERROR), "error") == 0, "error");
    CHECK(strcmp(tamis_severity_name((tamis_severity)99), "unknown") == 0, "sev inconnue");
    for (i = 0; i < (int)TAMIS_VERDICT__COUNT; i++) {
        tamis_severity s = tamis_verdict_default_severity((tamis_verdict_code)i);
        CHECK(s == EXPECT_SEV[i], "sévérité par défaut conforme au contrat");
        CHECK(s == TAMIS_SEV_INFO || s == TAMIS_SEV_WARNING || s == TAMIS_SEV_ERROR,
              "sévérité dans l'enum");
    }
}

static void test_uniqueness(void)
{
    int i, j;
    for (i = 0; i < (int)TAMIS_VERDICT__COUNT; i++) {
        for (j = i + 1; j < (int)TAMIS_VERDICT__COUNT; j++) {
            CHECK(strcmp(tamis_verdict_name((tamis_verdict_code)i),
                         tamis_verdict_name((tamis_verdict_code)j)) != 0,
                  "deux codes ont des noms distincts");
        }
    }
}

static void test_invalid(void)
{
    CHECK(tamis_verdict_code_valid(TAMIS_VERDICT__COUNT) == 0, "sentinelle invalide");
    CHECK(tamis_verdict_code_valid((tamis_verdict_code) -1) == 0, "négatif invalide");
    CHECK(tamis_verdict_code_valid((tamis_verdict_code)1000) == 0, "grand invalide");
    CHECK(strcmp(tamis_verdict_name(TAMIS_VERDICT__COUNT), "TAMIS_UNKNOWN") == 0,
          "nom hors plage -> TAMIS_UNKNOWN");
    CHECK(strcmp(tamis_verdict_name((tamis_verdict_code) -5), "TAMIS_UNKNOWN") == 0,
          "nom négatif -> TAMIS_UNKNOWN");
    CHECK(tamis_verdict_default_severity(TAMIS_VERDICT__COUNT) == TAMIS_SEV_INFO,
          "sévérité hors plage -> info");
    CHECK(tamis_verdict_default_severity((tamis_verdict_code) -1) == TAMIS_SEV_INFO,
          "sévérité négative -> info");
}

int main(void)
{
    test_names();
    test_severities();
    test_uniqueness();
    test_invalid();
    if (g_fail) {
        printf("FAILED test_verdict\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
