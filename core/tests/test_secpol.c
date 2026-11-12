/* SPDX-License-Identifier: MIT */
/* Tamis — tests du parseur secpol (core/src/secpol.c, secpol_lex.c). */
#include "tamis/secpol.h"
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

static char ARENA[262144];
static tamis_arena A;
static tamis_model M;
static tamis_verdict ERR;

/* Parse et renvoie le code ; le modèle et l'erreur restent dans M / ERR. */
static int parse(const char *texte)
{
    int rc;
    tamis_arena_init(&A, ARENA, sizeof ARENA);
    memset(&M, 0, sizeof M);
    memset(&ERR, 0, sizeof ERR);
    rc = tamis_model_parse_secpol(texte, &A, &M, &ERR);
    if (rc != 0 && ERR.message) {
        printf("     (parse refusé ligne %ld colonne %ld : %s)\n", ERR.line, ERR.column, ERR.message);
    }
    return rc;
}

static const char *src_of(size_t i) { return M.type_names[M.rules[i].src]; }
static const char *dst_of(size_t i) { return M.type_names[M.rules[i].dst]; }

static void test_minimal(void)
{
    int rc = parse("type a;\ntype b;\nallow a self:ability { settypeid:b; };\n");
    CHECK(rc == 0, "politique minimale acceptée");
    CHECK(M.n_types == 2, "2 types");
    CHECK(M.n_rules == 1, "1 règle");
    CHECK(strcmp(src_of(0), "a") == 0, "source a");
    CHECK(strcmp(dst_of(0), "b") == 0, "cible b");
    CHECK(M.rules[0].ability && strcmp(M.rules[0].ability, "settypeid") == 0, "ability settypeid");
    CHECK(M.rules[0].line == 3, "ligne de la règle");
}

static void test_ordre_source(void)
{
    int rc = parse("type a;\ntype b;\ntype c;\n"
                   "allow a self:ability { settypeid:b; };\n"
                   "allow b self:ability { settypeid:c; };\n");
    CHECK(rc == 0, "deux transitions acceptées");
    CHECK(M.n_rules == 2, "2 règles");
    CHECK(strcmp(src_of(0), "a") == 0 && strcmp(dst_of(0), "b") == 0, "règle 0 dans l'ordre source");
    CHECK(strcmp(src_of(1), "b") == 0 && strcmp(dst_of(1), "c") == 0, "règle 1 dans l'ordre source");
}

static void test_commentaires(void)
{
    int rc = parse("# un commentaire\ntype a; # en fin de ligne\n"
                   "type b;\nallow a self:ability {\n  # commentaire interne\n  settypeid:b;\n};\n");
    CHECK(rc == 0, "commentaires ignorés");
    CHECK(M.n_types == 2 && M.n_rules == 1, "modèle correct malgré les commentaires");
}

static void test_gain_priv(void)
{
    /* gain_priv sur une ability (non un type) : pas d'arête, pas d'erreur */
    int rc = parse("type a;\nallow a self:ability { gain_priv:pathspace; };\n");
    CHECK(rc == 0, "gain_priv sur une ability accepté");
    CHECK(M.n_rules == 0, "gain_priv sur une ability ne crée pas d'arête");
    /* gain_priv sur un type déclaré : arête */
    rc = parse("type a;\ntype b;\nallow a self:ability { gain_priv:b; };\n");
    CHECK(rc == 0, "gain_priv sur un type accepté");
    CHECK(M.n_rules == 1, "gain_priv sur un type crée une arête");
    /* gain_priv sur une permission custom : pas d'arête */
    rc = parse("type a;\ntype f1;\nallow a self:ability { gain_priv: file:read:f1; };\n");
    CHECK(rc == 0, "gain_priv sur une permission custom accepté");
    CHECK(M.n_rules == 0, "permission custom : pas d'arête");
}

static void test_abilities_et_plages(void)
{
    int rc = parse("type a;\nrange gpu_mem;\nattribute secure;\n"
                   "class file { read write };\n"
                   "ability network/bind/privport;\n"
                   "allow a self:ability {\n"
                   "  mem_phys:1024-4096,0x1200000-0x24000000\n"
                   "  setuid:4-6,23,96-\n"
                   "  interrupt:gpu_mem\n"
                   "  able_create\n"
                   "  network/bind/privport\n"
                   "};\n");
    CHECK(rc == 0, "abilities et plages acceptées");
    CHECK(M.n_types == 1, "1 type");
    CHECK(M.n_rules == 0, "aucune arête (pas de transition)");
}

static void test_channel_et_forms(void)
{
    int rc = parse("type a;\ntype b;\ntype c;\ntype d;\n"
                   "allow a b:channel connect;\n"
                   "allow_attach c /dev/socket/* d;\n"
                   "allow_link c /usr/lib/ldqnx-64.so.2;\n"
                   "derive_type a name b;\n"
                   "derive_type { a b } { settypeid:c } d;\n"
                   "default_spawn_type a b;\n"
                   "mount_point 1000:1001 {\n  settypeid:c\n};\n");
    CHECK(rc == 0, "toutes les formes acceptées");
    CHECK(M.n_rules == 1, "1 arête (canal)");
    CHECK(strcmp(src_of(0), "a") == 0 && strcmp(dst_of(0), "b") == 0, "arête de canal");
    CHECK(M.has_default_spawn == 1, "default_spawn_type détecté");
}

static void test_erreurs(void)
{
    int rc;
    rc = parse("type a\n");
    CHECK(rc == -1, "point-virgule manquant");
    CHECK(ERR.code == TAMIS_PARSE_ERROR, "code PARSE_ERROR");
    CHECK(ERR.line == 2 && ERR.column == 1, "position de l'erreur : fin de fichier (2:1)");
    rc = parse("type a;\ntype a;\n");
    CHECK(rc == -1, "type dupliqué refusé");
    rc = parse("allow x self:ability { settypeid:y; };\n");
    CHECK(rc == -1, "type source non déclaré refusé");
    CHECK(ERR.message && strstr(ERR.message, "source") != NULL, "message : type source");
    rc = parse("type a;\nallow a self:ability { settypeid:b; };\n");
    CHECK(rc == -1, "settypeid vers un type non déclaré refusé");
    rc = parse("type a;\ntype b;\nallow a b:ability { able_create };\n");
    CHECK(rc == -1, "cible d'ability non-self refusée");
    rc = parse("type a;\nallow a self:ability { setuid: };\n");
    CHECK(rc == -1, "argument d'ability manquant refusé");
    rc = parse("toto x;\n");
    CHECK(rc == -1, "mot-clé inconnu refusé");
    rc = parse("type a;\nallow a self:ability { able_create\n");
    CHECK(rc == -1, "accolade fermante manquante refusée");
    rc = parse("type a;\nallow a self:ability { able_create }\n");
    CHECK(rc == -1, "point-virgule final manquant refusé");
    rc = parse("type a;\nallow a self:ability { able_create };\n");
    CHECK(rc == 0, "point-virgule final présent accepté");
    rc = parse("type a;\nallow a self:ability { setuid:4; };\n");
    CHECK(rc == 0, "ability sans transition acceptée");
    CHECK(M.n_rules == 0, "sans transition : pas d'arête");
}

static void test_fixture_reelle(void)
{
    FILE *f = fopen("fixtures/politique-qnx8.secpol", "r");
    char buf[65536];
    size_t n;
    int rc;
    if (!f) {
        printf("INFO fixture absente : test sauté\n");
        g_checks++;
        return;
    }
    n = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[n] = '\0';
    rc = parse(buf);
    CHECK(rc == 0, "la politique QNX 8.0 réelle est acceptée");
    CHECK(M.n_types == 9, "9 types déclarés");
    CHECK(M.n_rules == 4, "4 arêtes (3 transitions + 1 canal)");
    if (M.n_rules == 4) {
        CHECK(strcmp(src_of(0), "app_tiers") == 0, "arête 0 : app_tiers");
        CHECK(strcmp(dst_of(0), "journal") == 0, "arête 0 : journal");
        CHECK(strcmp(dst_of(1), "diag") == 0, "arête 1 : diag");
        CHECK(strcmp(dst_of(2), "pompe_critique") == 0, "arête 2 : pompe_critique");
        CHECK(strcmp(src_of(3), "mm_client") == 0, "arête 3 : mm_client");
        CHECK(strcmp(dst_of(3), "mm_server") == 0, "arête 3 : mm_server");
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    test_minimal();
    test_ordre_source();
    test_commentaires();
    test_gain_priv();
    test_abilities_et_plages();
    test_channel_et_forms();
    test_erreurs();
    test_fixture_reelle();
    if (g_fail) {
        printf("FAILED test_secpol\n");
        return 1;
    }
    printf("OK %d checks\n", g_checks);
    return 0;
}
