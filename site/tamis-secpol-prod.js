"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — productions du langage secpol (QNX 8.0), côté navigateur.
 * Jumeau de core/src/secpol_parse.c et du pilote core/src/secpol.c : mêmes
 * productions, mêmes causes d'erreur, mêmes positions. Contrat : docs/SECPOL-PARSEUR.md.
 * window.TamisSecpolProd dans le navigateur, module.exports sous Node. */

/* Récupère les dépendances : modules TamisSecpol* (navigateur) ou require (Node). */
function deps() {
  if (typeof require === "function") {
    return {
      L: require("./tamis-secpol-lex.js"),
      A: require("./tamis-secpol-abil.js"),
      D: require("./tamis-secpol-decl.js"),
    };
  }
  return { L: window.TamisSecpolLex, A: window.TamisSecpolAbil, D: window.TamisSecpolDecl };
}
const { L, A, D } = deps();

/* ————— état du parseur ————— */
function nouvelEtat() {
  return {
    types: [], declare: new Map(), membres: new Map(), regles: [], sources: [],
    abilites: new Set(), classes: new Map(), attributs: new Set(), ranges: new Set(),
    relabels: new Map(), hasDefaultSpawn: false,
  };
}

/* Ajoute une règle (order source préservé) : src/dst sont des noms à ce stade. */
function ajouterRegle(etat, src, dst, ability, ligne, obligatoire) {
  etat.regles.push({ src, dst, ability, ligne, obligatoire });
}

/* ————— productions élémentaires ————— */

/* type <nom> [ , <attribut> … ] ; */
function pType(lx, etat) {
  const nom = L.attendreIdent(lx);
  if (D.typeDeclare(etat, nom.txt)) throw L.erreur(nom.ligne, 1, "type déjà déclaré");
  etat.declare.set(nom.txt, true);
  etat.types.push(nom.txt);
  for (;;) {
    lx.sauter();
    if (lx.cur() !== ",") break;
    lx.avancer(1);
    const attr = L.attendreIdent(lx);
    if (!D.attributDeclare(etat, attr.txt)) {
      throw L.erreur(attr.ligne, attr.colonne, "attribut non déclaré");
    }
    D.declarerMembre(etat, attr.txt, nom.txt);
  }
  L.attendrePt(lx, ";");
}

/* ability <nom> ; — nom brut (peut contenir « / »). */
function pAbilityDecl(lx, etat) {
  const mot = lx.brut(" \t\r\n;");
  if (mot.txt === "") throw L.erreur(lx.ligne, lx.colonne, "nom d'ability attendu");
  D.declarerAbility(etat, mot.txt, lx.ligne, lx.colonne);
  L.attendrePt(lx, ";");
}

/* attribute <nom> ; */
function pAttribute(lx, etat) {
  const attr = L.attendreIdent(lx);
  D.declarerAttribut(etat, attr.txt, attr.ligne, attr.colonne);
  L.attendrePt(lx, ";");
}

/* range <nom> ; — la re-déclaration est acceptée par le compilateur. */
function pRange(lx, etat) {
  const rg = L.attendreIdent(lx);
  D.declarerRange(etat, rg.txt);
  L.attendrePt(lx, ";");
}

/* class <nom> { permission… } ; */
function pClass(lx, etat) {
  const nom = L.attendreIdent(lx);
  D.declarerClasse(etat, nom.txt, nom.ligne, nom.colonne);
  L.attendrePt(lx, "{");
  for (;;) {
    lx.sauter();
    if (lx.fin()) throw L.erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
    if (lx.cur() === "}") { lx.avancer(1); break; }
    const perm = L.attendreIdent(lx);
    D.declarerPermission(etat, nom.txt, perm.txt, perm.ligne, perm.colonne);
  }
  L.attendrePt(lx, ";");
}

/* ————— abilities ————— */

/* Vérifie l'argument d'une ability NON-transition (miroir de verifier_arg_ability). */
function verifierArgAbility(etat, mot, arg, al, ac) {
  if (!A.abilSousPlage(mot) && !D.abilityDeclaree(etat, mot)) {
    throw L.erreur(al, ac, "cette ability n'accepte pas de sous-plage");
  }
  if (A.abilArgType(mot)) {
    if (!D.porteeAttribut(etat, arg)) throw L.erreur(al, ac, "type cible non déclaré");
    return;
  }
  if (arg[0] >= "0" && arg[0] <= "9") {
    const v = D.plageValide(arg);
    if (v === 0) throw L.erreur(al, ac, "plage mal formée");
    if (v === 2) throw L.erreur(al, ac, "borne de fin avant le début de la plage");
    return;
  }
  if (!D.rangeDeclaree(etat, arg)) throw L.erreur(al, ac, "plage nommée non déclarée");
}

/* Un élément d'ability : « mot » ou « mot:args ». */
function pAbilityItem(lx, etat, src, ligneRegle) {
  const mot = lx.brut(": \t\r\n;}");
  if (mot.txt === "") throw L.erreur(mot.ligne, mot.colonne, "ability vide");
  const transition = A.estTransition(mot.txt);
  if (!transition && !A.abilConnueStatique(mot.txt) && !D.abilityDeclaree(etat, mot.txt)) {
    throw L.erreur(mot.ligne, mot.colonne, "ability inconnue");
  }
  lx.sauter();
  if (lx.cur() === ":") {
    lx.avancer(1);
    for (;;) {
      const arg = lx.brut(", \t\r\n;}");
      if (arg.txt === "") throw L.erreur(arg.ligne, arg.colonne, "argument d'ability manquant");
      if (transition) {
        ajouterRegle(etat, src, arg.txt, mot.txt, ligneRegle, mot.txt === "settypeid");
      } else {
        verifierArgAbility(etat, mot.txt, arg.txt, arg.ligne, arg.colonne);
      }
      lx.sauter();
      if (lx.cur() === ",") { lx.avancer(1); continue; }
      break;
    }
  }
  /* Point-virgule facultatif en fin d'item (toléré, comme le compilateur). */
  lx.sauter();
  if (lx.cur() === ";") lx.avancer(1);
}

/* Bloc d'abilities entre accolades (le '{' est déjà consommé). */
function pBlocAbilities(lx, etat, src, ligneRegle) {
  for (;;) {
    lx.sauter();
    if (lx.fin()) throw L.erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
    if (lx.cur() === "}") { lx.avancer(1); return; }
    pAbilityItem(lx, etat, src, ligneRegle);
  }
}

/* ————— allow ————— */

/* allow <src> <dst> : ability { … } ;  |  : channel connect ;  |  : <classe> <perm> ; */
function pAllow(lx, etat) {
  lx.sauter();
  const ligneRegle = lx.ligne;
  const src = L.repere(lx);
  const dst = L.repere(lx);
  let j = lx.suivant();
  if (j.kind !== "ponct" || j.txt !== ":") {
    throw L.erreur(j.ligne, j.colonne, "« : » attendu après les types");
  }
  j = lx.suivant();
  if (j.kind !== "ident") throw L.erreur(j.ligne, j.colonne, "mot-clé attendu après « : »");
  if (j.txt === "ability") {
    if (!L.repereEstSelf(dst)) {
      throw L.erreur(ligneRegle, 0, "une règle d'abilities doit cibler « self »");
    }
    const jj = lx.suivant();
    if (jj.kind !== "ponct" || jj.txt !== "{") throw L.erreur(jj.ligne, jj.colonne, "« { » attendu");
    for (const s of src) {
      pBlocAbilities(lx, etat, s, ligneRegle);
      etat.sources.push({ nom: s, ligne: ligneRegle });  /* type source à valider */
    }
    L.attendrePt(lx, ";");
    return;
  }
  if (j.txt === "channel") {
    const connect = lx.suivant();
    if (connect.kind !== "ident" || connect.txt !== "connect") {
      throw L.erreur(connect.ligne, connect.colonne, "« connect » attendu après « channel »");
    }
    const fin = lx.suivant();
    if (fin.kind !== "ponct" || fin.txt !== ";") throw L.erreur(fin.ligne, fin.colonne, "« ; » attendu");
    for (const s of src) {
      for (const d of dst) {
        if (d === "self") continue;              /* « self » n'est pas un type */
        ajouterRegle(etat, s, d, "channel", ligneRegle, true);
      }
    }
    return;
  }
  /* Forme « allow X Y : <classe> <permission> ; » */
  const classe = j.txt;
  if (!D.classeDeclaree(etat, classe)) throw L.erreur(j.ligne, j.colonne, "classe non déclarée");
  const perm = lx.suivant();
  if (perm.kind !== "ident") throw L.erreur(perm.ligne, perm.colonne, "permission attendue après la classe");
  if (!D.permissionDeclaree(etat, classe, perm.txt)) {
    throw L.erreur(perm.ligne, perm.colonne, "permission non déclarée pour cette classe");
  }
  const fin = lx.suivant();
  if (fin.kind !== "ponct" || fin.txt !== ";") throw L.erreur(fin.ligne, fin.colonne, "« ; » attendu");
  for (const s of src) {
    for (const d of dst) {
      if (d === "self") continue;
      ajouterRegle(etat, s, d, classe, ligneRegle, true);
    }
  }
}

/* ————— attach / link / derive / spawn / mount ————— */

/* allow_attach <type> <chemin> [<type>] ;  |  allow_attach <type> { bloc } ; */
function pAllowAttach(lx, etat) {
  const type = L.attendreIdent(lx);
  if (!D.porteeAttribut(etat, type.txt)) throw L.erreur(type.ligne, type.colonne, "type non déclaré");
  lx.sauter();
  if (lx.cur() === "{") { lx.avancer(1); lx.sauterBloc(); L.attendrePt(lx, ";"); return; }
  lx.brut(" \t\r\n;{}");                      /* le chemin */
  lx.sauter();
  let cible = null, cl2 = type.ligne, cc2 = type.colonne;
  if (lx.cur() !== ";") {
    const c = L.attendreIdent(lx);
    if (!D.porteeAttribut(etat, c.txt)) throw L.erreur(c.ligne, c.colonne, "type de relabel non déclaré");
    cible = c.txt; cl2 = c.ligne; cc2 = c.colonne;
  }
  D.declarerRelabel(etat, type.txt, cible, cl2, cc2);
  L.attendrePt(lx, ";");
}

/* allow_link <type> <chemin> ;  |  allow_link <type> { bloc } ; */
function pAllowLink(lx, etat) {
  const type = L.attendreIdent(lx);
  if (!D.porteeAttribut(etat, type.txt)) throw L.erreur(type.ligne, type.colonne, "type non déclaré");
  lx.sauter();
  if (lx.cur() === "{") { lx.avancer(1); lx.sauterBloc(); L.attendrePt(lx, ";"); return; }
  lx.brut(" \t\r\n;{}");
  L.attendrePt(lx, ";");
}

/* derive_type A name B ;  |  derive_type { types } { abilities } C ; */
function pDeriveType(lx) {
  lx.sauter();
  if (lx.cur() === "{") {
    lx.avancer(1); lx.sauterBloc();
    const j = lx.suivant();
    if (j.kind !== "ponct" || j.txt !== "{") throw L.erreur(j.ligne, j.colonne, "« { » attendu");
    lx.avancer(1); lx.sauterBloc();
    L.attendreIdent(lx);
    L.attendrePt(lx, ";");
    return;
  }
  L.attendreIdent(lx);
  L.attendreIdent(lx);
  L.attendreIdent(lx);
  L.attendrePt(lx, ";");
}

/* default_spawn_type A B … ; — tous les types cités doivent être déclarés. */
function pDefaultSpawn(lx, etat) {
  const nom = L.attendreIdent(lx);
  if (!D.porteeAttribut(etat, nom.txt)) throw L.erreur(nom.ligne, nom.colonne, "type du spawn non déclaré");
  for (;;) {
    lx.sauter();
    if (lx.cur() === ";") break;
    const suivant = L.attendreIdent(lx);
    if (!D.porteeAttribut(etat, suivant.txt)) {
      throw L.erreur(suivant.ligne, suivant.colonne, "type du spawn non déclaré");
    }
  }
  etat.hasDefaultSpawn = true;
  L.attendrePt(lx, ";");
}

/* mount_point uid:gid [perms] [ { chemins… } ] ; */
function pMountPoint(lx) {
  let j = lx.suivant();
  if (j.kind !== "nombre" && j.kind !== "hexa") throw L.erreur(j.ligne, j.colonne, "uid attendu");
  j = lx.suivant();
  if (j.kind !== "ponct" || j.txt !== ":") throw L.erreur(j.ligne, j.colonne, "« : » attendu");
  j = lx.suivant();
  if (j.kind !== "nombre" && j.kind !== "hexa") throw L.erreur(j.ligne, j.colonne, "gid attendu");
  lx.brut(" \t\r\n;{");
  lx.sauter();
  if (lx.cur() === "{") { lx.avancer(1); lx.sauterBloc(); }
  L.attendrePt(lx, ";");
}

/* ————— pilote ————— */

const PRODUCTIONS = {
  type: (lx, etat) => pType(lx, etat),
  ability: (lx, etat) => pAbilityDecl(lx, etat),
  attribute: (lx, etat) => pAttribute(lx, etat),
  range: (lx, etat) => pRange(lx, etat),
  class: (lx, etat) => pClass(lx, etat),
  allow: (lx, etat) => pAllow(lx, etat),
  allow_attach: (lx, etat) => pAllowAttach(lx, etat),
  allow_link: (lx, etat) => pAllowLink(lx, etat),
  derive_type: (lx) => pDeriveType(lx),
  default_spawn_type: (lx, etat) => pDefaultSpawn(lx, etat),
  mount_point: (lx) => pMountPoint(lx),
};

function analyser(lx, etat) {
  for (;;) {
    lx.sauter();
    if (lx.fin()) return;
    const kw = lx.suivant();
    if (kw.kind === "fin") return;
    if (kw.kind !== "ident") throw L.erreur(kw.ligne, kw.colonne, "mot-clé de production attendu");
    const prod = PRODUCTIONS[kw.txt];
    if (!prod) throw L.erreur(kw.ligne, kw.colonne, "mot-clé de production inconnu");
    prod(lx, etat);
  }
}

/* ————— construction du modèle (miroir de construire/resoudre_regles) ————— */

/* Résout les règles en arêtes src/dst indexées, dans l'ordre source. */
function construireModele(etat) {
  const index = new Map();
  etat.types.forEach((t, i) => index.set(t, i));
  const regles = [];
  for (const r of etat.regles) {
    const srcs = D.resoudreTerme(etat, r.src);
    if (!srcs) throw L.erreur(r.ligne, 1, "type source non déclaré");
    if (r.dst === null || r.dst === undefined) continue;
    const dsts = D.resoudreTerme(etat, r.dst);
    if (!dsts) {
      if (r.obligatoire) throw L.erreur(r.ligne, 1, "type cible non déclaré");
      continue;                                /* ability sans type : pas d'arête */
    }
    for (const s of srcs) {
      for (const d of dsts) {
        regles.push({ src: index.get(s), dst: index.get(d), ability: r.ability, id: null, line: r.ligne });
      }
    }
  }
  for (const s of etat.sources) {
    if (!D.resoudreTerme(etat, s.nom)) throw L.erreur(s.ligne, 1, "type source non déclaré");
  }
  return {
    types: etat.types.slice(), idx: Object.fromEntries(index),
    rules: regles, untyped: [], hasDefaultSpawn: etat.hasDefaultSpawn,
  };
}

const TamisSecpolProd = { nouvelEtat, analyser, construireModele, PRODUCTIONS };
if (typeof module !== "undefined" && module.exports) module.exports = TamisSecpolProd;
if (typeof window !== "undefined") window.TamisSecpolProd = TamisSecpolProd;