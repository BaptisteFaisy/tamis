"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — vocabulaire fermé des abilities QNX 8.0, côté navigateur.
 * Jumeau de core/src/secpol_abil.c : mêmes tables, mêmes prédicats.
 * Source : abilities.html (SDP 8.0) + secpol_language.html (modificateurs).
 * window.TamisSecpolAbil dans le navigateur, module.exports sous Node. */

/* Les 62 abilities statiques de QNX 8.0 (ordre alphabétique du tableau officiel). */
const STATIQUES = [
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
];

/* Les 30 abilities qui acceptent un argument (sous-plage). */
const SOUS_PLAGE = [
  "channel_connect", "chroot", "clockset", "confset", "cpumode", "event",
  "interrupt", "io", "map_fixed", "mem_add", "mem_lock", "mem_peer", "mem_phys",
  "pgrp", "priority", "prot_exec", "prot_write_and_exec", "rlimit",
  "rlimit_peer", "session", "setgid", "settypeid", "setuid", "signal",
  "spawn_setgid", "spawn_setuid", "wait", "xprocess_debug", "xprocess_mem_read",
  "xprocess_query"
];

/* Modificateurs de bloc d'abilities (secpol_language.html). */
const MODIFIEURS = ["nonroot", "unlock", "noinherit"];

function dans(table, nom) { return table.indexOf(nom) >= 0; }

function abilConnueStatique(nom) {
  if (!nom) return false;
  return dans(STATIQUES, nom) || dans(MODIFIEURS, nom);
}
function abilSousPlage(nom) { return !!nom && dans(SOUS_PLAGE, nom); }
function abilArgType(nom) { return nom === "channel_connect"; }
function estTransition(nom) { return nom === "settypeid" || nom === "gain_priv"; }

const TamisSecpolAbil = {
  STATIQUES, SOUS_PLAGE, MODIFIEURS,
  abilConnueStatique, abilSousPlage, abilArgType, estTransition,
};
if (typeof module !== "undefined" && module.exports) module.exports = TamisSecpolAbil;
if (typeof window !== "undefined") window.TamisSecpolAbil = TamisSecpolAbil;