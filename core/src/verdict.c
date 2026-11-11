/* SPDX-License-Identifier: MIT */
/* Tamis — implémentation des verdicts (CONTRACT.md §3, §4). Cœur pur : tables const. */
#include "tamis/verdict.h"

/* Noms canoniques des 13 codes, indexés par tamis_verdict_code (CONTRACT §3). */
static const char *const VERDICT_NAMES[TAMIS_VERDICT__COUNT] = {
    "TAMIS_OK",
    "TAMIS_PARSE_ERROR",
    "TAMIS_UNTYPED_PROCESS",
    "TAMIS_MISSING_DEFAULT_SPAWN",
    "TAMIS_DEAD_RULE",
    "TAMIS_REDUNDANT_RULE",
    "TAMIS_OVERBROAD_ABILITY",
    "TAMIS_RANGE_INVALID",
    "TAMIS_PATH_FOUND",
    "TAMIS_INTERFERENCE_POSSIBLE",
    "TAMIS_PRIVILEGE_WIDENED",
    "TAMIS_PRIVILEGE_NARROWED",
    "TAMIS_INCONSISTENT",
};

/* Sévérités par défaut (CONTRACT.md §3). */
static const tamis_severity VERDICT_SEV[TAMIS_VERDICT__COUNT] = {
    TAMIS_SEV_INFO,    /* TAMIS_OK */
    TAMIS_SEV_ERROR,   /* TAMIS_PARSE_ERROR */
    TAMIS_SEV_WARNING, /* TAMIS_UNTYPED_PROCESS */
    TAMIS_SEV_WARNING, /* TAMIS_MISSING_DEFAULT_SPAWN */
    TAMIS_SEV_INFO,    /* TAMIS_DEAD_RULE */
    TAMIS_SEV_INFO,    /* TAMIS_REDUNDANT_RULE */
    TAMIS_SEV_WARNING, /* TAMIS_OVERBROAD_ABILITY */
    TAMIS_SEV_ERROR,   /* TAMIS_RANGE_INVALID */
    TAMIS_SEV_ERROR,   /* TAMIS_PATH_FOUND */
    TAMIS_SEV_ERROR,   /* TAMIS_INTERFERENCE_POSSIBLE */
    TAMIS_SEV_ERROR,   /* TAMIS_PRIVILEGE_WIDENED */
    TAMIS_SEV_INFO,    /* TAMIS_PRIVILEGE_NARROWED */
    TAMIS_SEV_ERROR,   /* TAMIS_INCONSISTENT */
};

int tamis_verdict_code_valid(tamis_verdict_code code)
{
    int c = (int)code;
    return (c >= 0 && c < (int)TAMIS_VERDICT__COUNT) ? 1 : 0;
}

const char *tamis_verdict_name(tamis_verdict_code code)
{
    if (!tamis_verdict_code_valid(code)) {
        return "TAMIS_UNKNOWN";
    }
    return VERDICT_NAMES[(int)code];
}

const char *tamis_severity_name(tamis_severity sev)
{
    switch (sev) {
    case TAMIS_SEV_INFO:    return "info";
    case TAMIS_SEV_WARNING: return "warning";
    case TAMIS_SEV_ERROR:   return "error";
    default:                return "unknown";
    }
}

tamis_severity tamis_verdict_default_severity(tamis_verdict_code code)
{
    if (!tamis_verdict_code_valid(code)) {
        return TAMIS_SEV_INFO;
    }
    return VERDICT_SEV[(int)code];
}
