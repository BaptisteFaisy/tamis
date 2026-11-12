/* SPDX-License-Identifier: MIT */
/* Tamis — vocabulaire des abilities QNX 8.0 (source : abilities.html du SDP 8.0).
 * INTERNE : table fermée des abilities statiques, des modificateurs et des sous-plages. */
#ifndef TAMIS_SECPOL_ABIL_H
#define TAMIS_SECPOL_ABIL_H

/* Vrai si `nom` est une ability statique ou un modificateur (nonroot/unlock/noinherit). */
int abil_connue_statique(const char *nom);
/* Vrai si l'ability accepte un argument (sous-plage numérique, nommée ou type). */
int abil_sous_plage(const char *nom);
/* Vrai si l'argument est un nom de type (channel_connect) plutôt qu'une plage. */
int abil_arg_type(const char *nom);

#endif /* TAMIS_SECPOL_ABIL_H */