/* SPDX-License-Identifier: MIT */
/* Pont C du différentiel : lit la sérialisation TAB/base64 de tests/gen_verdicts.py */
/* sur stdin, rend chaque verdict avec tamis_report_json (CONTRACT §5.2, §6). */
#define _POSIX_C_SOURCE 200809L
#include "tamis/report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LIST 16
#define MAX_ELEM 160

static int b64val(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

/* Décode base64 (avec padding '=') dans out ; retourne le nombre d'octets écrits. */
static size_t b64decode(const char *in, char *out, size_t outcap)
{
    int buf = 0, bits = 0;
    size_t o = 0;
    const char *p;
    for (p = in; *p && *p != '='; p++) {
        int v = b64val(*p);
        if (v < 0) {
            continue;
        }
        buf = (buf << 6) | v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            if (o < outcap) {
                out[o++] = (char)((buf >> bits) & 0xFF);
            }
        }
    }
    return o;
}

/* Décode un champ base64 ; "@" -> NULL (sentinelle None). Champ vide -> chaîne vide. */
static const char *dec_or_null(char *field, char *buf, size_t cap)
{
    size_t n;
    if (field[0] == '@') {
        return NULL;
    }
    n = b64decode(field, buf, cap - 1);
    buf[n] = '\0';
    return buf;
}

/* Découpage en place d'une liste base64 séparée par des virgules. */
static size_t dec_list(char *field, char storage[][MAX_ELEM], const char **ptrs, size_t max)
{
    size_t count = 0;
    char *tok;
    if (field[0] == '\0') {
        return 0;
    }
    tok = strtok(field, ",");
    while (tok && count < max) {
        size_t n = b64decode(tok, storage[count], MAX_ELEM - 1);
        storage[count][n] = '\0';
        ptrs[count] = storage[count];
        count++;
        tok = strtok(NULL, ",");
    }
    return count;
}

/* Découpage en place d'une ligne par tabulations ; retourne le nombre de champs. */
static int split_tab(char *line, char **fields, int max)
{
    int nf = 0;
    char *p = line;
    fields[nf++] = p;
    for (; *p && nf < max; p++) {
        if (*p == '\t') {
            *p = '\0';
            fields[nf++] = p + 1;
        }
    }
    return nf;
}

static void render_line(char *line)
{
    char *fields[10];
    tamis_verdict v;
    static char fb[256], mb[1024], sb[256], db[256];
    static char pstore[MAX_LIST][MAX_ELEM], rstore[MAX_LIST][MAX_ELEM];
    static const char *pptr[MAX_LIST], *rptr[MAX_LIST];
    char out[8192];
    long n;

    if (split_tab(line, fields, 10) < 10) {
        return;
    }
    memset(&v, 0, sizeof v);
    v.code = (tamis_verdict_code)atoi(fields[0]);
    v.sev = (tamis_severity)atoi(fields[1]);
    v.line = atol(fields[2]);
    v.column = atol(fields[3]);
    v.file = dec_or_null(fields[4], fb, sizeof fb);
    v.message = dec_or_null(fields[5], mb, sizeof mb);
    v.src = dec_or_null(fields[6], sb, sizeof sb);
    v.dst = dec_or_null(fields[7], db, sizeof db);
    v.path_len = dec_list(fields[8], pstore, pptr, MAX_LIST);
    v.path = v.path_len ? pptr : NULL;
    v.rules_len = dec_list(fields[9], rstore, rptr, MAX_LIST);
    v.rules = v.rules_len ? rptr : NULL;

    n = tamis_report_json(&v, 1, out, sizeof out);
    if (n > 0) {
        fwrite(out, 1, strlen(out), stdout);
    }
}

int main(void)
{
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    while ((len = getline(&line, &cap, stdin)) != -1) {
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (line[0] != '\0') {
            render_line(line);
        }
    }
    free(line);
    return 0;
}
