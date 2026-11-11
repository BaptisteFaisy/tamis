/* SPDX-License-Identifier: MIT */
/* Tamis — codes de verdict et structure d'un verdict. */
/* Contrat gelé : voir CONTRACT.md §5.2. Ne pas modifier sans mettre à jour le contrat. */
#ifndef TAMIS_VERDICT_H
#define TAMIS_VERDICT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Les 13 codes de verdict (CONTRACT.md §5.2 ; cahier des charges §5.2). */
typedef enum tamis_verdict_code {
    TAMIS_OK = 0,
    TAMIS_PARSE_ERROR,
    TAMIS_UNTYPED_PROCESS,
    TAMIS_MISSING_DEFAULT_SPAWN,
    TAMIS_DEAD_RULE,
    TAMIS_REDUNDANT_RULE,
    TAMIS_OVERBROAD_ABILITY,
    TAMIS_RANGE_INVALID,
    TAMIS_PATH_FOUND,
    TAMIS_INTERFERENCE_POSSIBLE,
    TAMIS_PRIVILEGE_WIDENED,
    TAMIS_PRIVILEGE_NARROWED,
    TAMIS_INCONSISTENT,
    TAMIS_VERDICT__COUNT /* sentinelle : ne jamais émettre */
} tamis_verdict_code;

/* Sévérité d'un verdict (CONTRACT.md §5.2). */
typedef enum tamis_severity {
    TAMIS_SEV_INFO = 0,
    TAMIS_SEV_WARNING,
    TAMIS_SEV_ERROR
} tamis_severity;

/*
 * Un verdict. Tous les pointeurs de chaîne sont EMPRUNTÉS (non possédés) :
 * l'appelant garantit leur validité pendant toute la durée du rendu.
 * `path` et `rules` sont des tableaux de `path_len` / `rules_len` chaînes
 * (NULL si non applicable). Aucune allocation, aucun état global : cœur pur.
 */
typedef struct tamis_verdict {
    tamis_verdict_code code;
    tamis_severity     sev;
    const char        *file;     /* fichier analysé ; NULL = inconnu */
    long               line;     /* 1-based ; 0 = inconnue */
    long               column;   /* 1-based ; 0 = inconnue */
    const char        *message;  /* cause lisible ; jamais NULL pour un verdict émis */
    const char        *src;      /* type source (verdicts de chemin) ; NULL sinon */
    const char        *dst;      /* type cible  (verdicts de chemin) ; NULL sinon */
    const char *const *path;     /* types intermédiaires ; NULL = absent */
    size_t             path_len; /* nombre d'éléments dans path */
    const char *const *rules;    /* identifiants de règles concernés ; NULL = absent */
    size_t             rules_len;/* nombre d'éléments dans rules */
} tamis_verdict;

/* Nom canonique d'un code (ex. "TAMIS_PATH_FOUND"). Hors plage -> "TAMIS_UNKNOWN". */
const char *tamis_verdict_name(tamis_verdict_code code);

/* Nom canonique d'une sévérité : "info" | "warning" | "error". */
const char *tamis_severity_name(tamis_severity sev);

/* Sévérité par défaut d'un code (CONTRACT.md §5.2). */
tamis_severity tamis_verdict_default_severity(tamis_verdict_code code);

/* Vrai (1) si code est dans [0, TAMIS_VERDICT__COUNT), faux (0) sinon. */
int tamis_verdict_code_valid(tamis_verdict_code code);

#ifdef __cplusplus
}
#endif
#endif /* TAMIS_VERDICT_H */
