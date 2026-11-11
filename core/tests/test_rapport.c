/* SPDX-License-Identifier: MIT */
/* Tamis — tests unitaires de rapport.c (CONTRACT.md §5). Binaire autonome. */
#include "tamis/report.h"
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

#define CHECK_STR(got, want, msg)                                           \
    do {                                                                    \
        g_checks++;                                                         \
        if (strcmp((got), (want)) != 0) {                                   \
            printf("FAILED ligne %d : %s\n  attendu : [%s]\n  obtenu  : [%s]\n", \
                   __LINE__, (msg), (want), (got));                         \
            g_fail = 1;                                                     \
        }                                                                   \
    } while (0)

static char BUF[16384];

/* Verdict canonique de chemin (CONTRACT §5.2). */
static tamis_verdict make_path(void)
{
    static const char *path[] = {"app_tiers", "journal", "diag", "pompe_critique"};
    static const char *rules[] = {"R112", "R87", "R9"};
    tamis_verdict v;
    memset(&v, 0, sizeof v);
    v.code = TAMIS_PATH_FOUND;
    v.sev = TAMIS_SEV_ERROR;
    v.src = "app_tiers";
    v.dst = "pompe_critique";
    v.path = path; v.path_len = 4;
    v.rules = rules; v.rules_len = 3;
    v.file = "policy.secpol"; v.line = 214; v.column = 0;
    v.message = "chemin de privilège atteignable";
    return v;
}

static tamis_verdict make_msg(tamis_verdict_code code, tamis_severity sev, const char *msg)
{
    tamis_verdict v;
    memset(&v, 0, sizeof v);
    v.code = code; v.sev = sev; v.message = msg;
    return v;
}

static void test_json_canonical(void)
{
    tamis_verdict v = make_path();
    long n = tamis_report_json(&v, 1, BUF, sizeof BUF);
    const char *want =
        "{\"code\":\"TAMIS_PATH_FOUND\",\"sev\":\"error\",\"src\":\"app_tiers\","
        "\"dst\":\"pompe_critique\",\"hops\":3,"
        "\"path\":[\"app_tiers\",\"journal\",\"diag\",\"pompe_critique\"],"
        "\"rules\":[\"R112\",\"R87\",\"R9\"],\"file\":\"policy.secpol\",\"line\":214,"
        "\"message\":\"chemin de privilège atteignable\"}\n";
    CHECK(n >= 0, "json retour >= 0");
    CHECK((size_t)n == strlen(BUF), "json retour == strlen");
    CHECK_STR(BUF, want, "JSON canonique octet pour octet");
}

static void test_json_omission(void)
{
    tamis_verdict v = make_msg(TAMIS_OK, TAMIS_SEV_INFO, "rien");
    tamis_report_json(&v, 1, BUF, sizeof BUF);
    CHECK_STR(BUF, "{\"code\":\"TAMIS_OK\",\"sev\":\"info\",\"message\":\"rien\"}\n",
              "clés optionnelles omises");
    CHECK(strstr(BUF, "src") == NULL, "pas de src");
    CHECK(strstr(BUF, "hops") == NULL, "pas de hops");
    CHECK(strstr(BUF, "path") == NULL, "pas de path");
    CHECK(strstr(BUF, "rules") == NULL, "pas de rules");
    CHECK(strstr(BUF, "file") == NULL, "pas de file");
    CHECK(strstr(BUF, "line") == NULL, "pas de line");
    CHECK(strstr(BUF, "column") == NULL, "pas de column");
}

static void test_json_column(void)
{
    tamis_verdict v = make_msg(TAMIS_PARSE_ERROR, TAMIS_SEV_ERROR, "jeton inattendu");
    v.file = "p.secpol"; v.line = 7; v.column = 12;
    tamis_report_json(&v, 1, BUF, sizeof BUF);
    CHECK_STR(BUF,
              "{\"code\":\"TAMIS_PARSE_ERROR\",\"sev\":\"error\",\"file\":\"p.secpol\","
              "\"line\":7,\"column\":12,\"message\":\"jeton inattendu\"}\n",
              "position file:line:column");
}

static void test_json_escaping(void)
{
    tamis_verdict v = make_msg(TAMIS_OK, TAMIS_SEV_INFO, "a\"b\\c\nd\te\bf\fg\rh");
    tamis_report_json(&v, 1, BUF, sizeof BUF);
    CHECK(strstr(BUF, "\\\"") != NULL, "guillemet échappé");
    CHECK(strstr(BUF, "\\\\") != NULL, "backslash échappé");
    CHECK(strstr(BUF, "\\n") != NULL, "\\n");
    CHECK(strstr(BUF, "\\t") != NULL, "\\t");
    CHECK(strstr(BUF, "\\b") != NULL, "\\b");
    CHECK(strstr(BUF, "\\f") != NULL, "\\f");
    CHECK(strstr(BUF, "\\r") != NULL, "\\r");
    CHECK(strstr(BUF, "\"a\\\"b\\\\c\\nd\\te\\bf\\fg\\rh\"") != NULL, "chaîne complète échappée");

    /* Caractère de contrôle < 0x20 -> \u00xx hex minuscule. */
    tamis_verdict c = make_msg(TAMIS_OK, TAMIS_SEV_INFO, "x\x01y\x1fz");
    tamis_report_json(&c, 1, BUF, sizeof BUF);
    CHECK(strstr(BUF, "\\u0001") != NULL, "contrôle 0x01 -> \\u0001");
    CHECK(strstr(BUF, "\\u001f") != NULL, "contrôle 0x1f -> \\u001f");

    /* UTF-8 non-ASCII passé tel quel (ensure_ascii=False). */
    tamis_verdict u = make_msg(TAMIS_OK, TAMIS_SEV_INFO, "sécurité—naïve");
    tamis_report_json(&u, 1, BUF, sizeof BUF);
    CHECK(strstr(BUF, "sécurité—naïve") != NULL, "UTF-8 non échappé");
    CHECK(strstr(BUF, "\\u00e9") == NULL, "pas de \\u pour é");
}

static void test_text(void)
{
    tamis_verdict v = make_path();
    long n = tamis_report_text(&v, 1, BUF, sizeof BUF);
    CHECK((size_t)n == strlen(BUF), "text retour == strlen");
    CHECK(strstr(BUF, "ERROR  TAMIS_PATH_FOUND  policy.secpol:214  "
                      "app_tiers -> pompe_critique (3 sauts) : R112,R87,R9\n") != NULL,
          "ligne de chemin exacte");
    CHECK(strstr(BUF, "FAILED 1 error(s)\n") != NULL, "synthèse FAILED");

    tamis_verdict w = make_msg(TAMIS_UNTYPED_PROCESS, TAMIS_SEV_WARNING, "processus non typé");
    w.file = "policy.secpol"; w.line = 88;
    tamis_report_text(&w, 1, BUF, sizeof BUF);
    CHECK(strstr(BUF, "WARN   TAMIS_UNTYPED_PROCESS  policy.secpol:88  processus non typé\n") != NULL,
          "ligne warning exacte");
    CHECK(strstr(BUF, "OK\n") != NULL, "synthèse OK sans error");

    tamis_verdict d = make_msg(TAMIS_DEAD_RULE, TAMIS_SEV_INFO, "règle jamais atteinte : R42");
    d.file = "policy.secpol"; d.line = 120;
    tamis_report_text(&d, 1, BUF, sizeof BUF);
    CHECK(strstr(BUF, "INFO   TAMIS_DEAD_RULE  policy.secpol:120  règle jamais atteinte : R42\n") != NULL,
          "ligne info exacte");
}

static void test_text_multi(void)
{
    tamis_verdict vs[3];
    vs[0] = make_path();
    vs[1] = make_msg(TAMIS_UNTYPED_PROCESS, TAMIS_SEV_WARNING, "processus non typé");
    vs[2] = make_msg(TAMIS_DEAD_RULE, TAMIS_SEV_INFO, "règle morte");
    tamis_report_text(vs, 3, BUF, sizeof BUF);
    CHECK(strstr(BUF, "FAILED 1 error(s)\n") != NULL, "1 error sur 3 verdicts");
    CHECK(tamis_report_exit_code(vs, 3) == 1, "exit_code 1 si error");
    tamis_report_text(&vs[1], 2, BUF, sizeof BUF);
    CHECK(strstr(BUF, "OK\n") != NULL, "OK sans error");
    CHECK(tamis_report_exit_code(&vs[1], 2) == 0, "exit_code 0 sans error");
}

static void test_markdown(void)
{
    tamis_verdict v = make_path();
    tamis_report_meta m = {"policy.secpol",
                           "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                           "2026-11-11T22:00:00Z", "0.1.0"};
    long n = tamis_report_markdown(&v, 1, &m, BUF, sizeof BUF);
    CHECK(n > 0, "markdown retour > 0");
    CHECK(strstr(BUF, "# Rapport d'audit Tamis\n") != NULL, "titre");
    CHECK(strstr(BUF, "- Politique : policy.secpol\n") != NULL, "politique");
    CHECK(strstr(BUF, "- Empreinte SHA-256 : 0123456789abcdef") != NULL, "empreinte");
    CHECK(strstr(BUF, "- Horodatage (UTC) : 2026-11-11T22:00:00Z\n") != NULL, "horodatage");
    CHECK(strstr(BUF, "- Version de Tamis : 0.1.0\n") != NULL, "version");
    CHECK(strstr(BUF, "## Synthèse\n") != NULL, "synthèse");
    CHECK(strstr(BUF, "| error | 1 |\n") != NULL, "compteur error");
    CHECK(strstr(BUF, "## Verdicts\n") != NULL, "section verdicts");
    CHECK(strstr(BUF, "### TAMIS_PATH_FOUND — error (×1)\n") != NULL, "section code");
    CHECK(strstr(BUF, "- Position : policy.secpol:214\n") != NULL, "position");
    CHECK(strstr(BUF, "- Chemin : app_tiers → pompe_critique (3 sauts) : "
                      "app_tiers → journal → diag → pompe_critique\n") != NULL, "chemin");
    CHECK(strstr(BUF, "- Règles : R112, R87, R9\n") != NULL, "règles");
    CHECK(strstr(BUF, "## Propriétés vérifiées\n") != NULL, "propriétés");
}

static void test_markdown_meta_null(void)
{
    tamis_verdict v = make_msg(TAMIS_OK, TAMIS_SEV_INFO, "ok");
    tamis_report_markdown(&v, 1, NULL, BUF, sizeof BUF);
    CHECK(strstr(BUF, "- Politique : inconnu\n") != NULL, "meta NULL -> politique inconnu");
    CHECK(strstr(BUF, "- Empreinte SHA-256 : inconnu\n") != NULL, "meta NULL -> empreinte inconnu");
    CHECK(strstr(BUF, "- Version de Tamis : inconnu\n") != NULL, "meta NULL -> version inconnu");
}

static void test_truncation(void)
{
    tamis_verdict v = make_path();
    long full = tamis_report_json(&v, 1, BUF, sizeof BUF);
    char small[16];
    long r = tamis_report_json(&v, 1, small, sizeof small);
    CHECK(r == full, "troncature : retour == longueur totale logique");
    CHECK(strlen(small) <= sizeof small - 1, "petit tampon NUL-terminé");
    CHECK(small[sizeof small - 1] == '\0', "dernier octet à NUL");
    /* tampon de 1 octet */
    char one[1];
    long r1 = tamis_report_json(&v, 1, one, 1);
    CHECK(r1 == full, "cap=1 : retour inchangé");
    CHECK(one[0] == '\0', "cap=1 : NUL");
    /* cap=0 : ne touche pas au tampon, retourne la longueur */
    long r0 = tamis_report_json(&v, 1, NULL, 0);
    CHECK(r0 == full, "cap=0,buf=NULL : retour == longueur");
}

static void test_invalid_args(void)
{
    tamis_verdict v = make_path();
    CHECK(tamis_report_json(NULL, 1, BUF, sizeof BUF) == -1, "json v=NULL,n>0 -> -1");
    CHECK(tamis_report_text(NULL, 1, BUF, sizeof BUF) == -1, "text v=NULL,n>0 -> -1");
    CHECK(tamis_report_markdown(NULL, 1, NULL, BUF, sizeof BUF) == -1, "md v=NULL,n>0 -> -1");
    CHECK(tamis_report_json(&v, 1, NULL, 100) == -1, "json buf=NULL,cap>0 -> -1");
    /* n=0 : valide, sortie vide */
    CHECK(tamis_report_json(&v, 0, BUF, sizeof BUF) == 0, "json n=0 -> 0");
    CHECK(BUF[0] == '\0', "json n=0 -> vide");
    CHECK(tamis_report_text(&v, 0, BUF, sizeof BUF) > 0, "text n=0 -> synthèse seule");
    CHECK(tamis_report_exit_code(NULL, 0) == 0, "exit_code n=0 -> 0");
}

static void test_all_codes(void)
{
    int i;
    for (i = 0; i < (int)TAMIS_VERDICT__COUNT; i++) {
        tamis_verdict_code c = (tamis_verdict_code)i;
        tamis_verdict v = make_msg(c, tamis_verdict_default_severity(c), "m");
        long n = tamis_report_json(&v, 1, BUF, sizeof BUF);
        CHECK(n > 0, "json code rendu");
        CHECK((size_t)n == strlen(BUF), "retour == strlen");
        CHECK(strstr(BUF, tamis_verdict_name(c)) != NULL, "nom présent");
        CHECK(strstr(BUF, tamis_severity_name(v.sev)) != NULL, "sev présente");
    }
}

static void test_json_multi(void)
{
    tamis_verdict vs[3];
    int lines = 0;
    char *p;
    long n;
    vs[0] = make_path();
    vs[1] = make_msg(TAMIS_DEAD_RULE, TAMIS_SEV_INFO, "r1");
    vs[2] = make_msg(TAMIS_OK, TAMIS_SEV_INFO, "r2");
    n = tamis_report_json(vs, 3, BUF, sizeof BUF);
    for (p = BUF; *p; p++) {
        if (*p == '\n') { lines++; }
    }
    CHECK(lines == 3, "3 lignes JSON (une par verdict)");
    CHECK((size_t)n == strlen(BUF), "retour == strlen multi");
    CHECK(BUF[n - 1] == '\n', "dernier octet = \\n");
}

int main(void)
{
    test_json_canonical();
    test_json_omission();
    test_json_column();
    test_json_escaping();
    test_text();
    test_text_multi();
    test_markdown();
    test_markdown_meta_null();
    test_truncation();
    test_invalid_args();
    test_all_codes();
    test_json_multi();
    if (g_fail) {
        printf("FAILED test_rapport\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
