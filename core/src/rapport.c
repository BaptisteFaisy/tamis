/* SPDX-License-Identifier: MIT */
/* Tamis — rendu des verdicts en texte, JSON et Markdown (CONTRACT.md §5). Cœur pur. */
#include "tamis/report.h"
#include <stdio.h>
#include <string.h>

/* Écrivain borné : suit la longueur logique totale (sémantique snprintf), */
/* NUL-termine le tampon quand il y a de la place, ne déborde jamais. */
typedef struct {
    char  *buf;
    size_t cap;
    size_t len; /* octets écrits logiquement, hors NUL */
} writer;

static void w_init(writer *w, char *buf, size_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->len = 0;
    if (buf && cap > 0) {
        buf[0] = '\0';
    }
}

static void w_add(writer *w, const char *s, size_t n)
{
    if (w->buf && w->cap > 0) {
        size_t room = (w->len < w->cap - 1) ? (w->cap - 1 - w->len) : 0;
        size_t cp = (n < room) ? n : room;
        size_t end;
        if (cp > 0) {
            memcpy(w->buf + w->len, s, cp);
        }
        end = w->len + cp;
        if (end > w->cap - 1) {
            end = w->cap - 1; /* borne la position du NUL, jamais hors tampon */
        }
        w->buf[end] = '\0';
    }
    w->len += n;
}

static void w_puts(writer *w, const char *s) { w_add(w, s, strlen(s)); }
static void w_putc(writer *w, char c) { w_add(w, &c, 1); }

static void w_long(writer *w, long v)
{
    char tmp[24];
    int n = snprintf(tmp, sizeof tmp, "%ld", v);
    if (n > 0) {
        w_add(w, tmp, (size_t)n);
    }
}

/* Chaîne JSON échappée (CONTRACT §5.2) : \b\f\n\r\t courts, autres <0x20 en \u00xx */
/* hexadécimal MINUSCULE, " et \ échappés, octets UTF-8 >= 0x20 passés tels quels. */
static void w_json_string(writer *w, const char *s)
{
    const unsigned char *p;
    w_putc(w, '"');
    for (p = (const unsigned char *)s; *p; p++) {
        unsigned char c = *p;
        switch (c) {
        case '"':  w_puts(w, "\\\""); break;
        case '\\': w_puts(w, "\\\\"); break;
        case '\b': w_puts(w, "\\b");  break;
        case '\f': w_puts(w, "\\f");  break;
        case '\n': w_puts(w, "\\n");  break;
        case '\r': w_puts(w, "\\r");  break;
        case '\t': w_puts(w, "\\t");  break;
        default:
            if (c < 0x20) {
                char esc[8];
                int n = snprintf(esc, sizeof esc, "\\u%04x", c);
                if (n > 0) {
                    w_add(w, esc, (size_t)n);
                }
            } else {
                w_putc(w, (char)c);
            }
        }
    }
    w_putc(w, '"');
}

static void w_json_array(writer *w, const char *const *arr, size_t n)
{
    size_t i;
    w_putc(w, '[');
    for (i = 0; i < n; i++) {
        if (i) {
            w_putc(w, ',');
        }
        w_json_string(w, arr[i] ? arr[i] : "");
    }
    w_putc(w, ']');
}

/* Émet « "clé": » avec la virgule si ce n'est pas le premier champ. */
static void jkey(writer *w, int *first, const char *key)
{
    if (!*first) {
        w_putc(w, ',');
    }
    *first = 0;
    w_json_string(w, key);
    w_putc(w, ':');
}

/* Un objet JSON de verdict — ordre des clés FIGÉ (CONTRACT §5.2). */
static void json_one(writer *w, const tamis_verdict *v)
{
    int first = 1;
    w_putc(w, '{');
    jkey(w, &first, "code"); w_json_string(w, tamis_verdict_name(v->code));
    jkey(w, &first, "sev");  w_json_string(w, tamis_severity_name(v->sev));
    if (v->src) { jkey(w, &first, "src"); w_json_string(w, v->src); }
    if (v->dst) { jkey(w, &first, "dst"); w_json_string(w, v->dst); }
    if (v->path_len > 0) {
        jkey(w, &first, "hops"); w_long(w, (long)v->path_len - 1);
        jkey(w, &first, "path"); w_json_array(w, v->path, v->path_len);
    }
    if (v->rules_len > 0) { jkey(w, &first, "rules"); w_json_array(w, v->rules, v->rules_len); }
    if (v->file) { jkey(w, &first, "file"); w_json_string(w, v->file); }
    if (v->line > 0) { jkey(w, &first, "line"); w_long(w, v->line); }
    if (v->column > 0) { jkey(w, &first, "column"); w_long(w, v->column); }
    jkey(w, &first, "message"); w_json_string(w, v->message ? v->message : "");
    w_putc(w, '}');
}

static const char *sev_label(tamis_severity sev)
{
    switch (sev) {
    case TAMIS_SEV_ERROR:   return "ERROR";
    case TAMIS_SEV_WARNING: return "WARN";
    default:                return "INFO";
    }
}

/* POSITION = file|? puis :line puis :column (CONTRACT §5.1). */
static void text_position(writer *w, const tamis_verdict *v)
{
    w_puts(w, v->file ? v->file : "?");
    if (v->line > 0) { w_putc(w, ':'); w_long(w, v->line); }
    if (v->column > 0) { w_putc(w, ':'); w_long(w, v->column); }
}

static void text_rules(writer *w, const tamis_verdict *v)
{
    size_t i;
    for (i = 0; i < v->rules_len; i++) {
        if (i) { w_putc(w, ','); }
        w_puts(w, v->rules[i] ? v->rules[i] : "");
    }
}

/* DÉTAIL = chemin si présent, sinon message (CONTRACT §5.1). */
static void text_detail(writer *w, const tamis_verdict *v)
{
    if (v->path_len > 0) {
        w_puts(w, v->src ? v->src : "?");
        w_puts(w, " -> ");
        w_puts(w, v->dst ? v->dst : "?");
        w_puts(w, " ("); w_long(w, (long)v->path_len - 1); w_puts(w, " sauts)");
        if (v->rules_len > 0) { w_puts(w, " : "); text_rules(w, v); }
    } else {
        w_puts(w, v->message ? v->message : "");
    }
}

static void text_one(writer *w, const tamis_verdict *v)
{
    char pad[8];
    int n = snprintf(pad, sizeof pad, "%-5s  ", sev_label(v->sev));
    if (n > 0) { w_add(w, pad, (size_t)n); }
    w_puts(w, tamis_verdict_name(v->code));
    w_puts(w, "  ");
    text_position(w, v);
    w_puts(w, "  ");
    text_detail(w, v);
    w_putc(w, '\n');
}

static size_t count_sev(const tamis_verdict *v, size_t n, tamis_severity sev)
{
    size_t i, c = 0;
    for (i = 0; i < n; i++) {
        if (v[i].sev == sev) { c++; }
    }
    return c;
}

long tamis_report_text(const tamis_verdict *v, size_t n, char *buf, size_t cap)
{
    writer w;
    size_t i, errors;
    if ((v == NULL && n > 0) || (buf == NULL && cap > 0)) {
        return -1;
    }
    w_init(&w, buf, cap);
    for (i = 0; i < n; i++) {
        text_one(&w, &v[i]);
    }
    errors = count_sev(v, n, TAMIS_SEV_ERROR);
    if (errors == 0) {
        w_puts(&w, "OK\n");
    } else {
        w_puts(&w, "FAILED "); w_long(&w, (long)errors); w_puts(&w, " error(s)\n");
    }
    return (long)w.len;
}

long tamis_report_json(const tamis_verdict *v, size_t n, char *buf, size_t cap)
{
    writer w;
    size_t i;
    if ((v == NULL && n > 0) || (buf == NULL && cap > 0)) {
        return -1;
    }
    w_init(&w, buf, cap);
    for (i = 0; i < n; i++) {
        json_one(&w, &v[i]);
        w_putc(&w, '\n');
    }
    return (long)w.len;
}

int tamis_report_exit_code(const tamis_verdict *v, size_t n)
{
    return (count_sev(v, n, TAMIS_SEV_ERROR) > 0) ? 1 : 0;
}

/* ————— Rendu Markdown d'audit (CONTRACT §5.3) ————— */

static void md_meta(writer *w, const tamis_report_meta *m)
{
    w_puts(w, "# Rapport d'audit Tamis\n\n");
    w_puts(w, "- Politique : ");
    w_puts(w, (m && m->policy_file) ? m->policy_file : "inconnu"); w_putc(w, '\n');
    w_puts(w, "- Empreinte SHA-256 : ");
    w_puts(w, (m && m->policy_sha256) ? m->policy_sha256 : "inconnu"); w_putc(w, '\n');
    w_puts(w, "- Horodatage (UTC) : ");
    w_puts(w, (m && m->timestamp_utc) ? m->timestamp_utc : "inconnu"); w_putc(w, '\n');
    w_puts(w, "- Version de Tamis : ");
    w_puts(w, (m && m->tamis_version) ? m->tamis_version : "inconnu"); w_puts(w, "\n\n");
}

static void md_summary(writer *w, const tamis_verdict *v, size_t n)
{
    w_puts(w, "## Synthèse\n\n| Sévérité | Nombre |\n|---|---|\n");
    w_puts(w, "| error | ");   w_long(w, (long)count_sev(v, n, TAMIS_SEV_ERROR));   w_puts(w, " |\n");
    w_puts(w, "| warning | "); w_long(w, (long)count_sev(v, n, TAMIS_SEV_WARNING)); w_puts(w, " |\n");
    w_puts(w, "| info | ");    w_long(w, (long)count_sev(v, n, TAMIS_SEV_INFO));    w_puts(w, " |\n\n");
}

static void md_bullets(writer *w, const tamis_verdict *v)
{
    size_t i;
    w_puts(w, "- Position : "); text_position(w, v); w_putc(w, '\n');
    w_puts(w, "- Message : "); w_puts(w, v->message ? v->message : ""); w_putc(w, '\n');
    if (v->path_len > 0) {
        w_puts(w, "- Chemin : ");
        w_puts(w, v->src ? v->src : "?"); w_puts(w, " → ");
        w_puts(w, v->dst ? v->dst : "?");
        w_puts(w, " ("); w_long(w, (long)v->path_len - 1); w_puts(w, " sauts) : ");
        for (i = 0; i < v->path_len; i++) {
            if (i) { w_puts(w, " → "); }
            w_puts(w, v->path[i] ? v->path[i] : "");
        }
        w_putc(w, '\n');
    }
    if (v->rules_len > 0) {
        w_puts(w, "- Règles : ");
        for (i = 0; i < v->rules_len; i++) {
            if (i) { w_puts(w, ", "); }
            w_puts(w, v->rules[i] ? v->rules[i] : "");
        }
        w_putc(w, '\n');
    }
}

static void md_code_section(writer *w, const tamis_verdict *v, size_t n, tamis_verdict_code code)
{
    size_t i, cnt = 0;
    for (i = 0; i < n; i++) {
        if (v[i].code == code) { cnt++; }
    }
    if (cnt == 0) {
        return;
    }
    w_puts(w, "### "); w_puts(w, tamis_verdict_name(code));
    w_puts(w, " — "); w_puts(w, tamis_severity_name(tamis_verdict_default_severity(code)));
    w_puts(w, " (×"); w_long(w, (long)cnt); w_puts(w, ")\n");
    for (i = 0; i < n; i++) {
        if (v[i].code == code) { md_bullets(w, &v[i]); }
    }
    w_putc(w, '\n');
}

long tamis_report_markdown(const tamis_verdict *v, size_t n,
                           const tamis_report_meta *meta, char *buf, size_t cap)
{
    writer w;
    int code;
    if ((v == NULL && n > 0) || (buf == NULL && cap > 0)) {
        return -1;
    }
    w_init(&w, buf, cap);
    md_meta(&w, meta);
    md_summary(&w, v, n);
    w_puts(&w, "## Verdicts\n\n");
    for (code = 0; code < (int)TAMIS_VERDICT__COUNT; code++) {
        md_code_section(&w, v, n, (tamis_verdict_code)code);
    }
    w_puts(&w, "## Propriétés vérifiées\n\n");
    w_puts(&w, "(une section par propriété : énoncé, méthode, corpus, résultat, contre-exemple)\n");
    return (long)w.len;
}
