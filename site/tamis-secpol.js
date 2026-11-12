"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — parseur du langage de politique de sécurité QNX 8.0 (secpol), côté navigateur.
 * Jumeau de core/src/secpol.c : mêmes productions, mêmes causes d'erreur, mêmes positions.
 * Contrat : docs/SECPOL-PARSEUR.md §2-§5 — grammaire gelée : docs/GRAMMAR.md.
 * DOM-free, sans dépendance : window.TamisSecpol dans le navigateur, module.exports sous Node.
 * Sous-ensemble STRICT : à la première erreur, { model: null, error: { line, column, message } }. */

const TRANSITIONS = ["settypeid", "gain_priv"];

/* ————————————————————— outils de texte ————————————————————— */

function erreur(ligne, colonne, message) {
  const e = new Error(message);
  e.tamisParse = true;
  e.ligne = ligne;
  e.colonne = colonne;
  return e;
}

function estDepartIdent(c) {
  return (c >= "a" && c <= "z") || (c >= "A" && c <= "Z") || c === "_";
}
function estIdent(c) { return estDepartIdent(c) || (c >= "0" && c <= "9"); }
function estChiffre(c) { return c >= "0" && c <= "9"; }
function estHexa(c) { return estChiffre(c) || (c >= "a" && c <= "f") || (c >= "A" && c <= "F"); }

/* Un nom : IDENT du contrat §2. */
function estNom(t) {
  if (t.length === 0 || !estDepartIdent(t[0])) return false;
  for (let i = 1; i < t.length; i++) if (!estIdent(t[i])) return false;
  return true;
}

/* Un nombre : décimal ou hexadécimal (0x…), avec au moins un chiffre. */
function estNombre(t) {
  if (t.length === 0) return false;
  if (t.length > 2 && t[0] === "0" && (t[1] === "x" || t[1] === "X")) {
    for (let i = 2; i < t.length; i++) if (!estHexa(t[i])) return false;
    return true;
  }
  for (let i = 0; i < t.length; i++) if (!estChiffre(t[i])) return false;
  return true;
}

/* Une plage : « N », « N- » (borne ouverte) ou « N-M ». */
function estPlage(t) {
  const i = t.indexOf("-");
  if (i < 0) return estNombre(t);
  const gauche = t.slice(0, i), droite = t.slice(i + 1);
  if (!estNombre(gauche)) return false;
  return droite === "" || estNombre(droite);
}

/* Un nom d'ability : IDENT, ou IDENT/IDENT… (network/bind/privport). */
function estMot(t) {
  const parts = t.split("/");
  for (let i = 0; i < parts.length; i++) if (!estNom(parts[i])) return false;
  return true;
}

/* ————————————————————— lexer ————————————————————— */

/* Balayage caractère par caractère (contrat §2) : pas de regex de découpage. */
class Lexer {
  constructor(texte) {
    this.t = typeof texte === "string" ? texte : "";
    this.p = 0;
    this.ligne = 1;
    this.colonne = 1;
  }
  fin() { return this.p >= this.t.length; }
  cur() { return this.p < this.t.length ? this.t[this.p] : ""; }
  avancer(n) {
    for (let i = 0; i < n && this.p < this.t.length; i++) {
      if (this.t[this.p] === "\n") { this.ligne++; this.colonne = 1; } else { this.colonne++; }
      this.p++;
    }
  }
  /* Espaces et commentaires '#' jusqu'à la fin de ligne. */
  sauter() {
    while (!this.fin()) {
      const c = this.cur();
      if (c === " " || c === "\t" || c === "\r" || c === "\n" || c === "\f" || c === "\v") {
        this.avancer(1);
      } else if (c === "#") {
        while (!this.fin() && this.cur() !== "\n") this.avancer(1);
      } else {
        return;
      }
    }
  }
  /* Jeton suivant : ident, nombre, ponctuation ; ou fin de fichier. */
  suivant() {
    this.sauter();
    const ligne = this.ligne, colonne = this.colonne;
    if (this.fin()) return { kind: "fin", txt: "", ligne: ligne, colonne: colonne };
    const c = this.cur();
    if (estDepartIdent(c)) {
      const debut = this.p;
      while (!this.fin() && estIdent(this.cur())) this.avancer(1);
      return { kind: "ident", txt: this.t.slice(debut, this.p), ligne: ligne, colonne: colonne };
    }
    if (estChiffre(c)) {
      const debut = this.p;
      const hexa = this.t[this.p] === "0" && (this.t[this.p + 1] === "x" || this.t[this.p + 1] === "X");
      if (hexa) {
        this.avancer(2);
        while (!this.fin() && estHexa(this.cur())) this.avancer(1);
      } else {
        while (!this.fin() && estChiffre(this.cur())) this.avancer(1);
      }
      return { kind: "nombre", txt: this.t.slice(debut, this.p), ligne: ligne, colonne: colonne };
    }
    if (c === ";" || c === "{" || c === "}" || c === ":") {
      this.avancer(1);
      return { kind: "ponct", txt: c, ligne: ligne, colonne: colonne };
    }
    throw erreur(ligne, colonne, "caractère inattendu");
  }
  /* Mot brut : jusqu'à un des arrêtants (chemins, ability_item, arguments). */
  brut(arretants) {
    this.sauter();
    const ligne = this.ligne, colonne = this.colonne;
    const debut = this.p;
    while (!this.fin() && arretants.indexOf(this.cur()) < 0) this.avancer(1);
    return { txt: this.t.slice(debut, this.p), ligne: ligne, colonne: colonne };
  }
  /* Bloc { … } imbriqué : le curseur est après l'accolade fermante. */
  sauterBloc() {
    let prof = 1;
    while (prof > 0) {
      this.sauter();
      if (this.fin()) throw erreur(this.ligne, this.colonne, "accolade fermante manquante");
      const c = this.cur();
      if (c === "{") { prof++; this.avancer(1); continue; }
      if (c === "}") { prof--; this.avancer(1); continue; }
      if (c === "#") { this.sauter(); continue; }
      this.avancer(1);
    }
  }
}

/* ————————————————————— aides du parseur ————————————————————— */

function attendreIdent(lx) {
  const j = lx.suivant();
  if (j.kind !== "ident") throw erreur(j.ligne, j.colonne, "identifiant attendu");
  return j;
}

function attendrePt(lx, c) {
  const msg = c === ";" ? "« ; » attendu" : (c === "{" ? "« { » attendu" : "« } » attendu");
  const j = lx.suivant();
  if (j.kind === "fin") throw erreur(j.ligne, j.colonne, "fin de fichier inattendue");
  if (j.kind !== "ponct" || j.txt !== c) throw erreur(j.ligne, j.colonne, msg);
}

/* Un repère de règle : un nom, ou un groupe { n1 n2 }. */
function parseRepere(lx) {
  lx.sauter();
  if (lx.cur() === "{") {
    lx.avancer(1);
    const noms = [];
    for (;;) {
      lx.sauter();
      if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
      if (lx.cur() === "}") { lx.avancer(1); break; }
      const mot = lx.brut(" \t\r\n;{}");
      if (mot.txt === "") throw erreur(mot.ligne, mot.colonne, "nom attendu dans le groupe");
      noms.push(mot.txt);
    }
    if (noms.length === 0) throw erreur(lx.ligne, lx.colonne, "nom attendu dans le groupe");
    return noms;
  }
  return [attendreIdent(lx).txt];
}

function repereEstSelf(noms) { return noms.length === 1 && noms[0] === "self"; }

/* Résout un terme (type ou attribut) en liste de noms de types. */
function resoudre(etat, terme) {
  if (etat.declare.has(terme)) return [terme];
  const membres = [];
  for (let i = 0; i < etat.membres.length; i++) {
    if (etat.membres[i].attr === terme) membres.push(etat.membres[i].membre);
  }
  return membres.length ? membres : null;
}

/* ————————————————————— productions ————————————————————— */

/* type <nom> [ , <attribut> … ] ; */
function parseType(lx, etat) {
  const nom = attendreIdent(lx);
  if (etat.declare.has(nom.txt)) throw erreur(nom.ligne, 1, "type déjà déclaré");
  etat.declare.add(nom.txt);
  etat.types.push(nom.txt);
  for (;;) {
    lx.sauter();
    if (lx.cur() !== ",") break;
    lx.avancer(1);
    const attr = attendreIdent(lx);
    etat.membres.push({ attr: attr.txt, membre: nom.txt });
  }
  attendrePt(lx, ";");
}

/* range IDENT ;  |  attribute IDENT ; */
function parseDeclSimple(lx) {
  attendreIdent(lx);
  attendrePt(lx, ";");
}

/* ability <nom> ; — déclaration d'ability dynamique (le nom peut contenir « / »). */
function parseAbilityDecl(lx) {
  const mot = lx.brut(" \t\r\n;");
  if (mot.txt === "") throw erreur(mot.ligne, mot.colonne, "nom d'ability attendu");
  attendrePt(lx, ";");
}

/* class IDENT { IDENT+ } ; */
function parseClass(lx) {
  attendreIdent(lx);
  attendrePt(lx, "{");
  let n = 0;
  for (;;) {
    lx.sauter();
    if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
    if (lx.cur() === "}") { lx.avancer(1); break; }
    attendreIdent(lx);
    n++;
  }
  if (n === 0) throw erreur(lx.ligne, lx.colonne, "permission attendue");
  attendrePt(lx, ";");
}

/* Une plage d'ability : NOMBRE, NOMBRE-NOMBRE, NOMBRE- ou un nom (§3). */
function validerArg(arg) {
  const t = arg.txt;
  if (t.indexOf(":") >= 0) {
    const parts = t.split(":");
    for (let i = 0; i < parts.length; i++) {
      if (parts[i] !== "*" && !estNom(parts[i])) {
        throw erreur(arg.ligne, arg.colonne, "argument d'ability mal formé : « " + t + " »");
      }
    }
    return;
  }
  if (estPlage(t) || estNom(t)) return;
  if (t.length > 1 && t[0] === "0" && (t[1] === "x" || t[1] === "X")) {
    throw erreur(arg.ligne, arg.colonne, "nombre hexadécimal sans chiffres : « " + t + " »");
  }
  throw erreur(arg.ligne, arg.colonne, "argument d'ability mal formé : « " + t + " »");
}

/* Un élément d'ability : MOT [ ':' liste_args ]. */
function parseAbilityItem(lx, transitions) {
  const mot = lx.brut(": \t\r\n;}");
  if (mot.txt === "") throw erreur(mot.ligne, mot.colonne, "ability vide");
  if (!estMot(mot.txt)) {
    throw erreur(mot.ligne, mot.colonne, "mot d'ability mal formé : « " + mot.txt + " »");
  }
  lx.sauter();
  if (lx.cur() === ":") {
    lx.avancer(1);
    for (;;) {
      const arg = lx.brut(", \t\r\n;}");
      if (arg.txt === "") throw erreur(arg.ligne, arg.colonne, "argument d'ability manquant");
      validerArg(arg);
      if (TRANSITIONS.indexOf(mot.txt) >= 0) {
        transitions.push({ dst: arg.txt, ability: mot.txt, obligatoire: mot.txt === "settypeid" });
      }
      lx.sauter();
      if (lx.cur() === ",") { lx.avancer(1); continue; }
      break;
    }
  }
  lx.sauter();
  if (lx.cur() === ";") lx.avancer(1);
}

/* bloc_ability : suite d'éléments jusqu'à '}'. */
function parseBloc(lx, transitions) {
  for (;;) {
    lx.sauter();
    if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
    if (lx.cur() === "}") { lx.avancer(1); return; }
    parseAbilityItem(lx, transitions);
  }
}

/* allow <src> <dst> : ability { … } ;  |  : channel connect ;  |  : <classe> <perm> ; */
function parseAllow(lx, etat) {
  lx.sauter();
  const ligneRegle = lx.ligne;              /* ligne du terme source (comme le C) */
  const src = parseRepere(lx);
  const dst = parseRepere(lx);
  lx.sauter();
  if (lx.cur() !== ":") throw erreur(lx.ligne, lx.colonne, "« : » attendu après les types");
  lx.avancer(1);
  const mot = lx.suivant();
  if (mot.kind !== "ident") throw erreur(mot.ligne, mot.colonne, "mot-clé attendu après « : »");
  if (mot.txt === "ability") {
    if (!repereEstSelf(dst)) {
      throw erreur(ligneRegle, 1, "une règle d'abilities doit cibler « self »");
    }
    const transitions = [];
    attendrePt(lx, "{");
    parseBloc(lx, transitions);
    attendrePt(lx, ";");
    for (let i = 0; i < src.length; i++) {
      for (let j = 0; j < transitions.length; j++) {
        ajouterRegle(etat, src[i], transitions[j].dst, transitions[j].ability,
                     ligneRegle, transitions[j].obligatoire);
      }
      etat.sources.push({ nom: src[i], ligne: ligneRegle });
    }
    return;
  }
  if (mot.txt === "channel") {
    const connect = lx.suivant();
    if (connect.kind !== "ident" || connect.txt !== "connect") {
      throw erreur(connect.ligne, connect.colonne, "« connect » attendu après « channel »");
    }
    attendrePt(lx, ";");
    for (let i = 0; i < src.length; i++) {
      for (let j = 0; j < dst.length; j++) {
        ajouterRegle(etat, src[i], dst[j], "channel", ligneRegle, true);
      }
    }
    return;
  }
  /* Permission de classe custom : allow X Y : <classe> <permission> ; */
  const perm = lx.suivant();
  if (perm.kind !== "ident") {
    throw erreur(perm.ligne, perm.colonne, "permission attendue après la classe");
  }
  attendrePt(lx, ";");
  for (let i = 0; i < src.length; i++) {
    for (let j = 0; j < dst.length; j++) {
      ajouterRegle(etat, src[i], dst[j], mot.txt, ligneRegle, true);
    }
  }
}

/* allow_attach IDENT CHEMIN [ CHEMIN ] ;  |  allow_attach IDENT { … } ; */
function parseChemin(lx, deuxChemins) {
  attendreIdent(lx);
  lx.sauter();
  if (lx.cur() === "{") {
    lx.avancer(1);
    lx.sauterBloc();
    attendrePt(lx, ";");
    return;
  }
  lx.brut(" \t\r\n;{}");
  lx.sauter();
  if (deuxChemins && lx.cur() !== ";") lx.brut(" \t\r\n;{}");
  attendrePt(lx, ";");
}

/* derive_type { NOM+ } { bloc } NOM ;  |  derive_type NOM NOM NOM ; */
function parseDeriveType(lx) {
  lx.sauter();
  if (lx.cur() !== "{") {
    attendreIdent(lx);
    attendreIdent(lx);
    attendreIdent(lx);
    attendrePt(lx, ";");
    return;
  }
  lx.avancer(1);
  lx.sauterBloc();
  attendrePt(lx, "{");
  lx.sauterBloc();
  attendreIdent(lx);
  attendrePt(lx, ";");
}

/* default_spawn_type NOM NOM ; */
function parseDefaultSpawn(lx, etat) {
  attendreIdent(lx);
  attendreIdent(lx);
  lx.sauter();
  if (lx.cur() !== ";") {
    const j = lx.suivant();
    throw erreur(j.ligne, j.colonne, "« ; » attendu");
  }
  lx.avancer(1);
  etat.hasDefaultSpawn = true;
}

/* mount_point NOMBRE : NOMBRE [ perms ] [ { … } ] ; */
function parseMountPoint(lx) {
  const uid = lx.suivant();
  if (uid.kind !== "nombre") throw erreur(uid.ligne, uid.colonne, "uid attendu");
  attendrePt(lx, ":");
  const gid = lx.suivant();
  if (gid.kind !== "nombre") throw erreur(gid.ligne, gid.colonne, "gid attendu");
  lx.brut(" \t\r\n;{");
  lx.sauter();
  if (lx.cur() === "{") {
    lx.avancer(1);
    lx.sauterBloc();
  }
  attendrePt(lx, ";");
}

/* ————————————————————— modèle ————————————————————— */

function ajouterRegle(etat, src, dst, ability, ligne, obligatoire) {
  etat.regles.push({ src: src, dst: dst, ability: ability, ligne: ligne, obligatoire: obligatoire });
}

/* Résolution finale : types déclarés -> arêtes, dans l'ordre source (contrat §4). */
function construireModele(etat) {
  const regles = [];
  for (let i = 0; i < etat.regles.length; i++) {
    const r = etat.regles[i];
    const srcs = resoudre(etat, r.src);
    if (!srcs) throw erreur(r.ligne, 1, "type source non déclaré");
    const dsts = resoudre(etat, r.dst);
    if (!dsts) {
      if (r.obligatoire) throw erreur(r.ligne, 1, "type cible non déclaré");
      continue;                       /* ability sans type : pas d'arête */
    }
    for (let u = 0; u < srcs.length; u++) {
      for (let v = 0; v < dsts.length; v++) {
        regles.push({ src: srcs[u], dst: dsts[v], ability: r.ability, id: null, line: r.ligne });
      }
    }
  }
  for (let i = 0; i < etat.sources.length; i++) {
    const s = etat.sources[i];
    if (!resoudre(etat, s.nom)) throw erreur(s.ligne, 1, "type source non déclaré");
  }
  return {
    types: etat.types.slice(),
    rules: regles,
    untyped: [],
    hasDefaultSpawn: etat.hasDefaultSpawn,
  };
}

/* ————————————————————— pilote ————————————————————— */

function parseProgramme(lx, etat) {
  for (;;) {
    lx.sauter();
    if (lx.fin()) return;
    const kw = lx.suivant();
    if (kw.kind === "fin") return;
    if (kw.kind !== "ident") {
      throw erreur(kw.ligne, kw.colonne, "mot-clé de production attendu");
    }
    switch (kw.txt) {
      case "type": parseType(lx, etat); break;
      case "range": case "attribute": parseDeclSimple(lx); break;
      case "ability": parseAbilityDecl(lx); break;
      case "class": parseClass(lx); break;
      case "allow": parseAllow(lx, etat); break;
      case "allow_attach": parseChemin(lx, true); break;
      case "allow_link": parseChemin(lx, false); break;
      case "derive_type": parseDeriveType(lx); break;
      case "default_spawn_type": parseDefaultSpawn(lx, etat); break;
      case "mount_point": parseMountPoint(lx); break;
      default:
        throw erreur(kw.ligne, kw.colonne, "mot-clé de production inconnu");
    }
  }
}

/* API publique : parseSecpol(texte) -> { model, error }. Jamais d'analyse partielle. */
function parseSecpol(texte) {
  try {
    const lx = new Lexer(texte);
    const etat = {
      types: [], declare: new Set(), membres: [], regles: [], sources: [],
      hasDefaultSpawn: false,
    };
    parseProgramme(lx, etat);
    return { model: construireModele(etat), error: null };
  } catch (e) {
    if (e && e.tamisParse) {
      return { model: null, error: { line: e.ligne, column: e.colonne, message: e.message } };
    }
    throw e;
  }
}

const TamisSecpol = { parseSecpol };
if (typeof module !== "undefined" && module.exports) module.exports = TamisSecpol;
if (typeof window !== "undefined") window.TamisSecpol = TamisSecpol;
