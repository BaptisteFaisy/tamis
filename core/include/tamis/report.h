/* SPDX-License-Identifier: MIT */
/* Tamis — rendu des verdicts en texte, JSON et Markdown d'audit. */
/* Contrat gelé : voir CONTRACT.md §5.3 (JSON) et §5.4 (Markdown). */
#ifndef TAMIS_REPORT_H
#define TAMIS_REPORT_H

#include <stddef.h>
#include "tamis/verdict.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Métadonnées du rapport, fournies par l'APPELANT (le cœur est pur : ni horloge
 * ni fichier ni socket). policy_sha256 = empreinte SHA-256 hexadécimale
 * (64 caractères) du fichier analysé ; timestamp_utc = horodatage ISO-8601 UTC ;
 * tamis_version = version de l'outil. Tout champ NULL est rendu « inconnu ».
 */
typedef struct tamis_report_meta {
    const char *policy_file;    /* nom du fichier analysé */
    const char *policy_sha256;  /* 64 hex minuscules ; NULL = non calculée */
    const char *timestamp_utc;  /* ISO-8601, ex. 2026-11-11T22:00:00Z ; NULL = inconnu */
    const char *tamis_version;  /* ex. "0.1.0" ; NULL = inconnu */
} tamis_report_meta;

/*
 * Chaque fonction écrit dans buf (cap octets) et retourne le nombre d'octets
 * qui AURAIENT été écrits tampon plein (sémantique snprintf). Retour < 0 :
 * argument invalide (v NULL et n>0, ou buf NULL avec cap>0). Le contenu est
 * toujours NUL-terminé quand cap > 0.
 */
long tamis_report_text    (const tamis_verdict *v, size_t n, char *buf, size_t cap);
long tamis_report_json    (const tamis_verdict *v, size_t n, char *buf, size_t cap);
long tamis_report_markdown(const tamis_verdict *v, size_t n,
                           const tamis_report_meta *meta, char *buf, size_t cap);

/* Code de retour processus : 1 si au moins un verdict de sévérité error, 0 sinon (CI). */
int tamis_report_exit_code(const tamis_verdict *v, size_t n);

#ifdef __cplusplus
}
#endif
#endif /* TAMIS_REPORT_H */
