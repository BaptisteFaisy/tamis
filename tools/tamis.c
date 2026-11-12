/* SPDX-License-Identifier: MIT */
/* tamis — CLI : analyse une politique (format provisoire) et rend les verdicts. */
/* Outil hôte : lit le fichier, calcule sha256/horodatage, pilote les moteurs du core. */
#define _POSIX_C_SOURCE 200809L
#include "tamis/analyze.h"
#include "tamis/report.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAMIS_VERSION "0.1.0"
#define ARENA_SZ (1u << 22)
#define MAX_VERDICTS 8192

typedef struct {
    const char *cmd, *file, *file2, *src, *dst, *format, *label;
} args;

/* Politique chargée : texte, arène, modèle construit, empreinte. */
typedef struct {
    char *text;
    size_t len;
    char *arena_buf;
    tamis_arena arena;
    tamis_model model;
    char sha[65];
} loaded;

static void usage(void)
{
    fputs(
        "tamis " TAMIS_VERSION " — analyse statique de politiques de sécurité QNX\n"
        "Usage : tamis <commande> <fichier> [options]\n"
        "Commandes : lint | reach | proof | minimize | diff <fichier2> | view | version\n"
        "Options : --src <type> --dst <type> --format text|json|md --label <nom>\n"
        "  reach/proof/explain : --src et --dst requis. diff : <fichier>=base, <fichier2>=candidat.\n"
        "  --label : nom rapporté dans les verdicts (défaut : le chemin du fichier).\n"
        "  view : émet le graphe de privilèges en JSON (pour le visualiseur HTML).\n",
        stderr);
}

static char *read_file(const char *path, size_t *len_out)
{
    FILE *f = fopen(path, "rb");
    char *buf;
    long sz;
    if (!f) {
        fprintf(stderr, "KO impossible de lire %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    *len_out = fread(buf, 1, (size_t)sz, f);
    buf[*len_out] = '\0';
    fclose(f);
    return buf;
}

static void now_iso(char out[32])
{
    time_t t = time(NULL);
    struct tm *g = gmtime(&t);
    strftime(out, 32, "%Y-%m-%dT%H:%M:%SZ", g);
}

static int parse_args(int argc, char **argv, args *a)
{
    int i;
    memset(a, 0, sizeof *a);
    a->format = "text";
    if (argc < 2) {
        return -1;
    }
    a->cmd = argv[1];
    for (i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--src") && i + 1 < argc) { a->src = argv[++i]; }
        else if (!strcmp(argv[i], "--dst") && i + 1 < argc) { a->dst = argv[++i]; }
        else if (!strcmp(argv[i], "--format") && i + 1 < argc) { a->format = argv[++i]; }
        else if (!strcmp(argv[i], "--label") && i + 1 < argc) { a->label = argv[++i]; }
        else if (argv[i][0] != '-') {
            if (!a->file) { a->file = argv[i]; } else { a->file2 = argv[i]; }
        } else { fprintf(stderr, "KO option inconnue : %s\n", argv[i]); return -1; }
    }
    return (strcmp(a->cmd, "version") && !a->file) ? -1 : 0;
}

/* Nom rapporté dans les verdicts : --label s'il est fourni, sinon le chemin du fichier. */
static const char *nom_analyse(const args *a)
{
    return a->label ? a->label : a->file;
}

static void render(const tamis_verdict *v, size_t n, const args *a, const char *sha)
{
    char ts[32];
    tamis_report_meta meta;
    long need;
    char *buf;
    int md = !strcmp(a->format, "md");
    now_iso(ts);
    meta.policy_file = nom_analyse(a);
    meta.policy_sha256 = sha;
    meta.timestamp_utc = ts;
    meta.tamis_version = TAMIS_VERSION;
    if (!strcmp(a->format, "json")) {
        need = tamis_report_json(v, n, NULL, 0);
    } else if (md) {
        need = tamis_report_markdown(v, n, &meta, NULL, 0);
    } else {
        need = tamis_report_text(v, n, NULL, 0);
    }
    buf = malloc((size_t)need + 1);
    if (!buf) {
        return;
    }
    if (!strcmp(a->format, "json")) {
        tamis_report_json(v, n, buf, (size_t)need + 1);
    } else if (md) {
        tamis_report_markdown(v, n, &meta, buf, (size_t)need + 1);
    } else {
        tamis_report_text(v, n, buf, (size_t)need + 1);
    }
    fputs(buf, stdout);
    free(buf);
}

static int run_engine(const args *a, const tamis_model *m, const tamis_model *m2, tamis_out *o)
{
    if (!strcmp(a->cmd, "lint")) { return tamis_lint(o, m); }
    if (!strcmp(a->cmd, "minimize")) { return tamis_minimize(o, m); }
    if (!strcmp(a->cmd, "reach")) { return tamis_reach(o, m, a->src, a->dst); }
    if (!strcmp(a->cmd, "proof")) { return tamis_proof(o, m, a->src, a->dst); }
    if (!strcmp(a->cmd, "explain")) { return tamis_explain(o, m, a->src, a->dst); }
    if (!strcmp(a->cmd, "diff")) { return tamis_diff(o, m, m2); }
    return -1;
}

/* Graphe de privilèges en JSON pour le visualiseur HTML (TAM-23). */
static void view_json(const tamis_model *m, const args *a)
{
    size_t i;
    printf("{\"nodes\":[");
    for (i = 0; i < m->n_types; i++) {
        printf("%s\"%s\"", i ? "," : "", m->type_names[i]);
    }
    printf("],\"edges\":[");
    for (i = 0; i < m->n_rules; i++) {
        const tamis_rule *r = &m->rules[i];
        printf("%s{\"src\":\"%s\",\"dst\":\"%s\",\"rule\":\"%s\"}", i ? "," : "",
               m->type_names[r->src], m->type_names[r->dst],
               r->id ? r->id : (r->ability ? r->ability : ""));
    }
    printf("]");
    if (a->src && a->dst) {
        printf(",\"focus\":{\"src\":\"%s\",\"dst\":\"%s\"}", a->src, a->dst);
    }
    printf("}\n");
}

/* Charge un fichier et construit le modèle. 0 ok, -1 syntaxe (*err rempli), -2 IO. */
static int load(const char *path, loaded *L, tamis_verdict *err)
{
    int rc;
    memset(L, 0, sizeof *L);
    L->text = read_file(path, &L->len);
    if (!L->text) {
        return -2;
    }
    tamis_sha256_hex(L->text, L->len, L->sha);
    L->arena_buf = malloc(ARENA_SZ);
    if (!L->arena_buf) {
        free(L->text);
        L->text = NULL;
        return -2;
    }
    tamis_arena_init(&L->arena, L->arena_buf, ARENA_SZ);
    rc = tamis_model_parse(L->text, &L->arena, &L->model, err);
    if (rc != 0) {
        err->file = path;
    }
    return rc;
}

static void loaded_free(loaded *L)
{
    free(L->text);
    free(L->arena_buf);
}

static int cmd_view(const args *a)
{
    loaded L;
    tamis_verdict err;
    int rc = load(a->file, &L, &err);
    if (rc == -2) {
        return 2;
    }
    if (rc == -1) {
        render(&err, 1, a, L.sha);
        loaded_free(&L);
        return 1;
    }
    view_json(&L.model, a);
    loaded_free(&L);
    return 0;
}

static int cmd_engine(const args *a)
{
    loaded L;
    tamis_verdict err, *vs;
    tamis_out o;
    int rc;
    if ((!strcmp(a->cmd, "reach") || !strcmp(a->cmd, "proof") || !strcmp(a->cmd, "explain")) &&
        (!a->src || !a->dst)) {
        fprintf(stderr, "KO %s exige --src et --dst\n", a->cmd);
        return 2;
    }
    rc = load(a->file, &L, &err);
    if (rc == -2) {
        return 2;
    }
    if (rc == -1) {
        render(&err, 1, a, L.sha);
        loaded_free(&L);
        return 1;
    }
    vs = calloc(MAX_VERDICTS, sizeof *vs);
    tamis_out_init(&o, vs, MAX_VERDICTS, &L.arena, nom_analyse(a));
    rc = run_engine(a, &L.model, NULL, &o);
    if (rc < 0) {
        fprintf(stderr, "KO moteur '%s' en erreur (type inconnu ?)\n", a->cmd);
        free(vs);
        loaded_free(&L);
        return 1;
    }
    render(o.v, o.n, a, L.sha);
    rc = tamis_report_exit_code(o.v, o.n);
    free(vs);
    loaded_free(&L);
    return rc;
}

static int cmd_diff(const args *a)
{
    loaded base, cand;
    tamis_verdict err, *vs;
    tamis_out o;
    int rc;
    if (!a->file2) {
        fprintf(stderr, "KO diff exige un second fichier\n");
        return 2;
    }
    rc = load(a->file, &base, &err);
    if (rc == -2) {
        return 2;
    }
    if (rc == -1) {
        render(&err, 1, a, base.sha);
        loaded_free(&base);
        return 1;
    }
    rc = load(a->file2, &cand, &err);
    if (rc != 0) {
        if (rc == -1) {
            render(&err, 1, a, cand.sha);
        }
        loaded_free(&base);
        loaded_free(&cand);
        return rc == -1 ? 1 : 2;
    }
    vs = calloc(MAX_VERDICTS, sizeof *vs);
    tamis_out_init(&o, vs, MAX_VERDICTS, &base.arena, nom_analyse(a));
    tamis_diff(&o, &base.model, &cand.model);
    render(o.v, o.n, a, base.sha);
    rc = tamis_report_exit_code(o.v, o.n);
    free(vs);
    loaded_free(&base);
    loaded_free(&cand);
    return rc;
}

int main(int argc, char **argv)
{
    args a;
    if (parse_args(argc, argv, &a) != 0) {
        usage();
        return 2;
    }
    if (!strcmp(a.cmd, "version")) {
        printf("tamis %s\n", TAMIS_VERSION);
        return 0;
    }
    if (!strcmp(a.cmd, "view")) {
        return cmd_view(&a);
    }
    if (!strcmp(a.cmd, "diff")) {
        return cmd_diff(&a);
    }
    if (!strcmp(a.cmd, "lint") || !strcmp(a.cmd, "reach") ||
        !strcmp(a.cmd, "proof") || !strcmp(a.cmd, "explain") || !strcmp(a.cmd, "minimize")) {
        return cmd_engine(&a);
    }
    usage();
    return 2;
}
