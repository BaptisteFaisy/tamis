/* SPDX-License-Identifier: MIT */
/* Tamis — productions du langage secpol (parser descendant récursif). Voir secpol.c. */
#include "secpol_priv.h"
#include "secpol_abil.h"
#include <string.h>

/* Vérifie l'argument d'une ability NON-transition (définie plus bas). */
static int verifier_arg_ability(parseur *p, const char *mot, const char *arg, long al, long ac);
/* Chemin + relabel optionnel d'un allow_attach (définie plus bas). */
static int p_chemin_relabel(parseur *p, char *type, long lg, long cl);

/* Enregistre une règle d'allow SANS arête : elle sert à valider que les types référencés
   sont bien déclarés (le compilateur refuse « undefined type » même si la règle ne crée
   aucune transition). dst = NULL marque une simple validation. */
static int ajouter_regle_valide(parseur *p, char *src, long ligne)
{
    regle_lien *r = tamis_arena_alloc(p->a, sizeof *r);
    if (!r) { return -2; }
    r->src = src;
    r->dst = NULL;
    r->ability = NULL;
    r->ligne = ligne;
    r->type_obligatoire = 1;
    r->suivant = p->regles;
    p->regles = r;
    p->n_regles++;
    return 0;
}

/* Un élément d'ability : « mot » ou « mot:args ». Les transitions (settypeid, gain_priv)
   portent le graphe de privilège ; les autres portent une ability et sont vérifiées contre
   le vocabulaire fermé de QNX 8.0 (abilities.html). */
int p_ability_item(parseur *p, char *src, long ligne_regle)
{
    long lg = 0, cl = 0, al = 0, ac = 0;
    char *mot = lx_brut(&p->lx, p->a, ": \t\r\n;}", &lg, &cl);
    int transition;
    if (mot[0] == '\0') {
        return echec(p, lg, cl, "ability vide");
    }
    transition = est_transition(mot);
    if (!transition && !abil_connue_statique(mot) && !ability_declaree(p, mot)) {
        return echec(p, lg, cl, "ability inconnue");
    }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == ':') {
        lx_avance(&p->lx, 1);
        for (;;) {
            char *arg = lx_brut(&p->lx, p->a, ", \t\r\n;}", &al, &ac);
            int rc;
            if (arg[0] == '\0') {
                return echec(p, al, ac, "argument d'ability manquant");
            }
            if (transition) {
                /* settypeid : la cible DOIT être un type déclaré ; gain_priv : si la cible
                   n'est pas un type, c'est une ability (le compilateur n'en fait qu'un warning). */
                rc = ajouter_regle(p, src, arg, mot, ligne_regle,
                                   strcmp(mot, "settypeid") == 0);
            } else {
                rc = verifier_arg_ability(p, mot, arg, al, ac);
            }
            if (rc != 0) { return rc; }
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

/* Vérifie l'argument d'une ability NON-transition : sous-plage numérique, plage nommée
   (range X;), ou type (channel_connect). Refuse les arguments sur une ability sans
   sous-plage, et les plages nommées non déclarées. */
static int verifier_arg_ability(parseur *p, const char *mot, const char *arg, long al, long ac)
{
    if (!abil_sous_plage(mot) && !ability_declaree(p, mot)) {
        return echec(p, al, ac, "cette ability n'accepte pas de sous-plage");
    }
    if (abil_arg_type(mot)) {
        return portee_attribut(p, arg) ? 0
               : echec(p, al, ac, "type cible non déclaré");
    }
    if (arg[0] >= '0' && arg[0] <= '9') {
        int v = plage_valide(arg);
        if (v == 0) { return echec(p, al, ac, "plage mal formée"); }
        if (v == 2) { return echec(p, al, ac, "borne de fin avant le début de la plage"); }
        return 0;
    }
    return range_declaree(p, arg) ? 0 : echec(p, al, ac, "plage nommée non déclarée");
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

/* Saute un bloc { … } (imbrication comprise) ; le curseur est après l'accolade fermante.
   Le contenu est lu en mode BRUT : un bloc peut contenir des chemins (/dev/random) que le
   découpage mot-à-mot ne sait pas lire. */
int p_sauter_bloc(parseur *p)
{
    int prof = 1;
    while (prof > 0) {
        if (p->lx.p >= p->lx.fin) {
            return echec(p, p->lx.ligne, p->lx.colonne, "accolade fermante manquante");
        }
        if (*p->lx.p == '{') { prof++; }
        else if (*p->lx.p == '}') { prof--; }
        lx_avance(&p->lx, 1);
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
            /* La règle doit référencer des types DÉCLARÉS, même sans transition. */
            rc = ajouter_regle_valide(p, s->nom, lg);
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
                int rc;
                if (strcmp(d->nom, "self") == 0) {
                    continue; /* « self » n'est pas un type : aucune arête nouvelle */
                }
                rc = ajouter_regle(p, s->nom, d->nom, "channel", lg, 1);
                if (rc != 0) { return rc; }
            }
        }
        return 0;
    }
    /* Forme « allow X Y : <classe> <permission> ; » (permission de classe custom). */
    {
        char *classe = j.txt;
        char *perm = NULL;
        long pl = 0, pc = 0;
        if (!classe_declaree(p, classe)) {
            return echec(p, j.ligne, j.colonne, "classe non déclarée");
        }
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_IDENT) {
            return echec(p, j.ligne, j.colonne, "permission attendue après la classe");
        }
        perm = j.txt;
        pl = j.ligne; pc = j.colonne;
        if (!permission_declaree(p, classe, perm)) {
            return echec(p, pl, pc, "permission non déclarée pour cette classe");
        }
        lx_suivant(&p->lx, p->a, &j);
        if (j.kind != J_PUNCT || j.txt[0] != ';') {
            return echec(p, j.ligne, j.colonne, "« ; » attendu");
        }
        for (s = src; s; s = s->suivant) {
            for (d = dst; d; d = d->suivant) {
                int rc;
                if (strcmp(d->nom, "self") == 0) {
                    continue; /* « self » n'est pas un type : aucune arête nouvelle */
                }
                rc = ajouter_regle(p, s->nom, d->nom, classe, lg, 1);
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
    if (!portee_attribut(p, type)) {
        return echec(p, lg, cl, "type non déclaré");
    }
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p == '{') {
        lx_avance(&p->lx, 1);
        if (p_sauter_bloc(p) != 0) { return -1; }
        return attendre_pt(p, ';');
    }
    return p_chemin_relabel(p, type, lg, cl);
}

/* Chemin d'un allow_attach, avec relabel optionnel : <chemin> [<type>]. Consigne le
   relabel pour contrôler la cohérence entre les règles d'un même type (I24). */
static int p_chemin_relabel(parseur *p, char *type, long lg, long cl)
{
    char *cible = NULL;
    long cl2 = 0, cc2 = 0;
    int rc;
    lx_brut(&p->lx, p->a, " \t\r\n;{}", NULL, NULL);   /* le chemin */
    lx_sauter(&p->lx);
    if (p->lx.p < p->lx.fin && *p->lx.p != ';') {
        if (attendre_ident(p, &cible, &cl2, &cc2) != 0) { return -1; }
        if (!portee_attribut(p, cible)) {
            return echec(p, cl2, cc2, "type de relabel non déclaré");
        }
    }
    rc = ajouter_relabel(p, type, cible, cible ? cl2 : lg, cible ? cc2 : cl);
    if (rc != 0) { return rc; }
    return attendre_pt(p, ';');
}

/* allow_link <type> <chemin> ;  |  allow_link <type> { bloc } ; */
int p_allow_link(parseur *p)
{
    char *type = NULL;
    long lg = 0, cl = 0;
    if (attendre_ident(p, &type, &lg, &cl) != 0) { return -1; }
    if (!portee_attribut(p, type)) {
        return echec(p, lg, cl, "type non déclaré");
    }
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
    int rc;
    if (attendre_ident(p, &nom, &lg, &cl) != 0) { return -1; }
    rc = ajouter_classe(p, nom);
    if (rc == -3) { return echec(p, lg, cl, "classe déjà déclarée"); }
    if (rc != 0) { return rc; }
    lx_suivant(&p->lx, p->a, &j);
    if (j.kind != J_PUNCT || j.txt[0] != '{') {
        return echec(p, j.ligne, j.colonne, "« { » attendu");
    }
    for (;;) {
        char *perm = NULL;
        lx_sauter(&p->lx);
        if (p->lx.p >= p->lx.fin) {
            return echec(p, p->lx.ligne, p->lx.colonne, "accolade fermante manquante");
        }
        if (*p->lx.p == '}') { lx_avance(&p->lx, 1); break; }
        if (attendre_ident(p, &perm, &lg, &cl) != 0) { return -1; }
        if (permission_declaree(p, nom, perm)) {
            return echec(p, lg, cl, "permission déjà déclarée pour cette classe");
        }
        rc = ajouter_permission(p, nom, perm);
        if (rc != 0) { return rc; }
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

/* default_spawn_type A B ;  (le nombre de types est libre, tous doivent être déclarés) */
int p_default_spawn(parseur *p)
{
    long lg = 0, cl = 0;
    char *nom = NULL;
    if (attendre_ident(p, &nom, &lg, &cl) != 0) { return -1; }
    if (!portee_attribut(p, nom)) {
        return echec(p, lg, cl, "type du spawn non déclaré");
    }
    for (;;) {
        lx_sauter(&p->lx);
        if (p->lx.p < p->lx.fin && *p->lx.p == ';') { break; }
        if (attendre_ident(p, &nom, &lg, &cl) != 0) { return -1; }
        if (!portee_attribut(p, nom)) {
            return echec(p, lg, cl, "type du spawn non déclaré");
        }
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

