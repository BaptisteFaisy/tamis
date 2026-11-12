/* SPDX-License-Identifier: MIT */
/* Tamis — vocabulaire fermé des abilities QNX 8.0.
 *
 * Source : abilities.html (SDP 8.0) — tableau « Static abilities ». La colonne
 * « Subrange (optional) » distingue les abilities qui acceptent un argument de
 * celles qui n'en acceptent pas : c'est la source des deux tables ci-dessous.
 *
 * Les modificateurs (nonroot, unlock, noinherit) et l'ability spéciale gain_priv
 * ne figurent pas dans abilities.html : ils sont documentés dans secpol_language.html.
 */
#include "secpol_abil.h"
#include <string.h>

/* Les 62 abilities statiques de QNX 8.0, dans l'ordre alphabétique du tableau officiel. */
static const char *const STATIQUES[] = {
    "able_create", "able_priv", "channel_connect", "child_newapp", "chroot",
    "clockset", "confset", "connection", "cpumode", "default_timer_tolerance",
    "event", "fork", "getid", "high_resolution_timer", "hyp", "interrupt", "io",
    "mac_policy", "map_fixed", "mem_add", "mem_lock", "mem_peer", "mem_phys",
    "mem_special", "mountifs", "msg_queue", "pathspace", "path_trust", "pgrp",
    "power", "priority", "privreg", "prot_exec", "prot_write_and_exec",
    "public_channel", "rconstraint", "reboot", "rlimit", "rlimit_peer",
    "rsrcdbmgr", "sandbox", "schedule", "server_monitor", "session", "setgid",
    "settypeid", "setuid", "sigev_thread", "signal", "spawn", "spawn_setgid",
    "spawn_setuid", "srandom", "swap", "trace", "umask", "untrusted_exec", "wait",
    "xprocess_able", "xprocess_debug", "xprocess_mem_read", "xprocess_query"
};

/* Les 30 abilities qui acceptent un argument (sous-plage) : colonne « Subrange »
 * non vide dans abilities.html. */
static const char *const SOUS_PLAGE[] = {
    "channel_connect", "chroot", "clockset", "confset", "cpumode", "event",
    "interrupt", "io", "map_fixed", "mem_add", "mem_lock", "mem_peer", "mem_phys",
    "pgrp", "priority", "prot_exec", "prot_write_and_exec", "rlimit",
    "rlimit_peer", "session", "setgid", "settypeid", "setuid", "signal",
    "spawn_setgid", "spawn_setuid", "wait", "xprocess_debug", "xprocess_mem_read",
    "xprocess_query"
};

/* Modificateurs de bloc d'abilities (secpol_language.html). */
static const char *const MODIFIEURS[] = { "nonroot", "unlock", "noinherit" };

static int dans_table(const char *const *tab, size_t n, const char *nom)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (strcmp(tab[i], nom) == 0) { return 1; }
    }
    return 0;
}

int abil_connue_statique(const char *nom)
{
    if (!nom) { return 0; }
    if (dans_table(STATIQUES, sizeof STATIQUES / sizeof *STATIQUES, nom)) { return 1; }
    return dans_table(MODIFIEURS, sizeof MODIFIEURS / sizeof *MODIFIEURS, nom);
}

int abil_sous_plage(const char *nom)
{
    if (!nom) { return 0; }
    return dans_table(SOUS_PLAGE, sizeof SOUS_PLAGE / sizeof *SOUS_PLAGE, nom);
}

int abil_arg_type(const char *nom)
{
    /* channel_connect : « Allowable channel type IDs » — des NOMS de types, pas des
       numéros. Les autres abilities à sous-plage prennent des plages numériques ou
       des plages nommées (range X;). */
    return nom && strcmp(nom, "channel_connect") == 0;
}
