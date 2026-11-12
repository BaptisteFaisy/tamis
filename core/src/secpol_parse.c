/* SPDX-License-Identifier: MIT */
/* Tamis — productions du langage secpol (parser descendant récursif). Voir secpol.c. */
#include "secpol_priv.h"
#include <string.h>

/* Un élément d'ability : « mot » ou « mot:args ». Les transitions (settypeid, gain_priv)
   portent le graphe de privilège ; les autres ne portent qu'une ability. */
int p_ability_item(parseur *p, char *src, long ligne_regle)
{
    long lg = 0, cl = 0, al = 0, ac = 0;
    char *mot = lx_brut(&p->lx, p->a, ": \t\r\n;}", &lg, &cl);
    if (mot[0] == '\0') {
        return echec(p, lg, cl, "ability vide");
    }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == ':') {
        lx_avance(&p->lx, 1);
        for (;;) {
            char *arg = lx_brut(&p->lx, p->a, ", \t\r\n;}", &al, &ac);
            if (arg[0] == '\0') {
                return echec(p, al, ac, "argument d'ability manquant");
            }
            if (est_transition(mot)) {
                /* settypeid : la cible DOIT être un type déclaré ; gain_priv : si la cible
                   n'est pas un type, c'est une ability (le compilateur n'en fait qu'un warning). */
                int rc = ajouter_regle(p, src, arg, mot, ligne_regle,
                                        strcmp(mot, "settypeid") == 0);
                if (rc != 0) { return rc; }
            }
            lx_sauter(&p->lx);
            if (p->lx.p < p->lx.fin && *p->lx.p == ',') {
                lx_avance(&p->lx, 1);
                continue;
            }
            break;
        }
    }
    /* Un point-virgule peut terminer l'item ; la doc officielle ne le met pas, mais
       l'accepter évite de rejeter des politiques écrites autrement. */
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == ';') {
        lx_avance(&p->lx, 1);
    }
    return 0;
}

/* Bloc d'abilities entre accolades (déjà après le '{'). */
int p_bloc_abilities(parseur *p, char *src, long ligne_regle)
{
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p >= p->lx.fin) {
            return echec(p, p->lx.ligne, p->lx.colonne, "accolade fermante manquante");
        }
        if (*p->lx.p == '}') { lx_avance(&p->lx, 1); return 0; }
        if (p_ability_item(p, src, ligne_regle) != 0) { return -1; }
    }
}

/* Saute un bloc { … } (imbrication comprise) ; le curseur est après l'accolade fermante. */
int p_sauter_bloc(parseur *p)
{
    jeton j;
    int prof = 1;
    while (prof > 0) {
        if (lx_suivant(&p->lx, p->a, &j) != 0) {
            return echec(p, p->lx.ligne, p->lx.colonne, "caractère inattendu");
        }
        if (j.kind == J_EOF) {
            return echec(p, j.ligne, j.colonne, "accolade fermante manquante");
        }
        if (j.kind == J_PUNCT && j.txt[0] == '{') { prof++; }
        else if (j.kind == J_PUNCT && j.txt[0] == '}') { prof--; }
    }
    return 0;
}

nom_lien *nom_lien_ajoute(parseur *p, const char *nom, nom_lien *tete)
{
    nom_lien *n = tamis_arena_alloc(p->a, sizeof *n);
    if (!n) {
        return NULL;
    }
    n->nom = (char *)nom;
    n->suivant = tete;
    return n;
}

/* Parse un repère : un nom simple, ou un groupe { nom… }. NULL si erreur. */
nom_lien *p_repere(parseur *p)
{
    nom_lien *tete = NULL;
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        for (;;) {
            char *nom;
            lx_sauter(&p->lx);
            if (p->lx.p >= p->lx.fin) {
                echec(p, p->lx.ligne, p->lx.colonne, "accolade fermante manquante");
                return NULL;
            }
            if (*p->lx.p == '}') {
                lx_avance(&p->lx, 1);
                return tete;
            }
            nom = lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);
            if (nom[0] == '\0') {
                echec(p, p->lx.ligne, p->lx.colonne, "nom attendu dans le groupe");
                return NULL;
            }
            tete = nom_lien_ajoute(p, nom, tete);
            if (!tete) {
                return NULL;
            }
        }
    }
    {
        jeton j;
        if (lx_suivant(&p->lx, p->a, &j) != 0 || j.kind != J_IDENT) {
            echec(p, p->lx.ligne, p->lx.colonne, "nom de type attendu");
            return NULL;
        }
        return nom_lien_ajoute(p, j.txt, NULL);
    }
}

/* Vrai si le repère est exactement « self ». */
int repere_est_self(const nom_lien *r)
{
    return r && r->suivant == NULL && strcmp(r->nom, "self") == 0;
}

/*
 * allow <src> <dst> : ability { … } ;      (la cible doit être « self »)
 * allow <src> <dst> : channel connect ;    (accès au canal)
 * allow <src> <dst> : <classe> <perm> ;    (permission de classe custom)
 * <src> et <dst> sont un nom ou un groupe { n1 n2 }.
 */
int p_allow(parseur *p)
{
    jeton j;
    nom_lien *src, *dst, *s, *d;
    long lg = 0, cl = 0;
    lx_sauter(&p->lx);
    lg = p->lx.ligne;
    src = p_repere(p);
    if (!src) { return -1; }
    dst = p_repere(p);
    if (!dst) { return -1; }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != ':') {
        return echec(p, j.ligne, j.colonne, "« : » attendu après les types");
    }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_IDENT) { return echec(p, j.ligne, j.colonne, "mot-clé attendu après « : »"); }
    if (strcmp(j.txt, "ability") == 0) {
        int rc;
        if (!repere_est_self(dst)) {
            return echec(p, lg, cl, "une règle d'abilities doit cibler « self »");
        }
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != '{') {
            return echec(p, j.ligne, j.colonne, "« { » attendu");
        }
        for (s = src; s; s = s->suivant) {
            rc = p_bloc_abilities(p, s->nom, lg);
            if (rc != 0) { return rc; }
        }
        return attendre_pt(p, ';');
    }
    if (strcmp(j.txt, "channel") == 0) {
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_IDENT || strcmp(j.txt, "connect") != 0) {
            return echec(p, j.ligne, j.colonne, "« connect » attendu après « channel »");
        }
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != ';') {
            return echec(p, j.ligne, j.colonne, "« ; » attendu");
        }
        for (s = src; s; s = s->suivant) {
            for (d = dst; d; d = d->suivant) {
                int rc = ajouter_regle(p, s->nom, d->nom, "channel", lg, 1);
                if (rc != 0) { return rc; }
            }
        }
        return 0;
    }
    /* Forme « allow X Y : <classe> <permission> ; » (permission de classe custom). */
    {
        char *classe = j.txt;
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_IDENT) {
            return echec(p, j.ligne, j.colonne, "permission attendue après la classe");
        }
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != ';') {
            return echec(p, j.ligne, j.colonne, "« ; » attendu");
        }
        for (s = src; s; s = s->suivant) {
            for (d = dst; d; d = d->suivant) {
                int rc = ajouter_regle(p, s->nom, d->nom, classe, lg, 1);
                if (rc != 0) { return rc; }
            }
        }
    }
    return 0;
}

/* allow_attach <type> <chemin> [<type>] ;  |  allow_attach <type> { bloc } ; */
int p_allow_attach(parseur *p)
{
    char *type = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &type, &lg, &cl) != 0) { return -1; }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }
        return attendre_pt(p, ';');
    }
    lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p != ';') {
        lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);
    }
    return attendre_pt(p, ';');
}

/* allow_link <type> <chemin> ;  |  allow_link <type> { bloc } ; */
int p_allow_link(parseur *p)
{
    char *type = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &type, &lg, &cl) != 0) { return -1; }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }
        return attendre_pt(p, ';');
    }
    lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);
    return attendre_pt(p, ';');
}

/* class <nom> { permission… } ; */
int p_class(parseur *p)
{
    jeton j;
    char *nom = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &nom, &lg, &cl) != 0) { return -1; }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != '{') {
        return echec(p, j.ligne, j.colonne, "« { » attendu");
    }
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p >= p->lx.fin) {
            return echec(p, p->lx.ligne, p->lx.colonne, "accolade fermante manquante");
        }
        if (*p->lx.p == '}') { lx_avance(&p->lx, 1); break; }
        if (lx_suivant(&p->lx, p->a, &j) != 0 || j.kind != J_IDENT) {
            return echec(p, p->lx.ligne, p->lx.colonne, "permission attendue");
        }
    }
    return attendre_pt(p, ';');
}

/* derive_type A name B ;  |  derive_type { types } { abilities } C ; */
int p_derive_type(parseur *p)
{
    jeton j;
    long lg = 0, cl = 0;
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }   /* les types */
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != '{') {
            return echec(p, j.ligne, j.colonne, "« { » attendu");
        }
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }   /* les abilities */
        if (attendre_ident(p, NULL, NULL, NULL) != 0) { return -1; }
        return attendre_pt(p, ';');
    }
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    return attendre_pt(p, ';');
}

/* default_spawn_type A B ;  (le nombre de types est libre) */
int p_default_spawn(parseur *p)
{
    long lg = 0, cl = 0;
    if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p < p->lx.fin && *p->lx.p == ';') { break; }
        if (attendre_ident(p, NULL, &lg, &cl) != 0) { return -1; }
    }
    p->default_spawn = 1;
    return attendre_pt(p, ';');
}

/* mount_point uid:gid [perms] [ { chemins… } ] ; */
int p_mount_point(parseur *p)
{
    jeton j;
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_NUM && j.kind != J_HEX) {
        return echec(p, j.ligne, j.colonne, "uid attendu");
    }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != ':') {
        return echec(p, j.ligne, j.colonne, "« : » attendu");
    }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_NUM && j.kind != J_HEX) {
        return echec(p, j.ligne, j.colonne, "gid attendu");
    }
    /* permissions optionnelles (ex. rw-rw-r--), puis bloc de chemins optionnel */
    lx_brut(&p->lx, p->a, " \t\r\n;{", NULL, NULL);
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }
    }
    return attendre_pt(p, ';');
}

