"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — parseur du langage de politique de sécurité QNX 8.0 (secpol), côté navigateur.
 * Jumeau de core/src/secpol.c : mêmes productions, mêmes causes d'erreur.
 * Contrat : docs/SECPOL-PARSEUR.md §2-§5 — grammaire gelée : docs/GRAMMAR.md.
 * DOM-free, sans dépendance : window.TamisSecpol dans le navigateur, module.exports sous Node.
 * Sous-ensemble STRICT : à la première erreur, on renvoie { model: null, error: {…} }. */

const MODIFICATEURS = ["nonroot", "unlock", "noinherit", "inherit", "deny", "allow", "root", "subrange"];
const TRANSITIONS = ["settypeid", "gain_priv"];
const RE_MOT = /^[A-Za-z_][A-Za-z0-9_]*(\/[A-Za-z_][A-Za-z0-9_]*)*$/;
const RE_NOM = /^[A-Za-z_][A-Za-z0-9_]*$/;
const RE_NUM = /^(0[xX][0-9a-fA-F]+|[0-9]+)$/;
const RE_PLAGE = /^(0[xX][0-9a-fA-F]+|[0-9]+)(-(0[xX][0-9a-fA-F]+|[0-9]+)?)?$/;

/* Erreur interne : porte la position (1-based) et la cause. */
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

/* ————————————————————— lexer ————————————————————— */

/* Balayage caractère par caractère : pas de regex de découpage (contrat §2). */
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
  /* Espaces + commentaires '#' jusqu'à la fin de ligne (contrat §2). */
  skipEspaces() {
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
  lireIdent() {
    this.skipEspaces();
    const ligne = this.ligne, colonne = this.colonne;
    if (!estDepartIdent(this.cur())) return null;
    const debut = this.p;
    while (!this.fin() && estIdent(this.cur())) this.avancer(1);
    return { texte: this.t.slice(debut, this.p), ligne: ligne, colonne: colonne };
  }
  lireNombre() {
    this.skipEspaces();
    const ligne = this.ligne, colonne = this.colonne;
    if (!estChiffre(this.cur())) return null;
    const debut = this.p;
    if (this.cur() === "0" && (this.t[this.p + 1] === "x" || this.t[this.p + 1] === "X")) {
      this.avancer(2);
      while (!this.fin() && estHexa(this.cur())) this.avancer(1);
    } else {
      while (!this.fin() && estChiffre(this.cur())) this.avancer(1);
    }
    return { texte: this.t.slice(debut, this.p), ligne: ligne, colonne: colonne };
  }
  attendre(c) {
    this.skipEspaces();
    if (this.cur() !== c) throw erreur(this.ligne, this.colonne, "« " + c + " » attendu");
    this.avancer(1);
  }
  /* Mot brut : jusqu'à un des arrêtants (chemins, ability_item, arguments). */
  lireBrut(arretants) {
    this.skipEspaces();
    const ligne = this.ligne, colonne = this.colonne;
    const debut = this.p;
    while (!this.fin() && arretants.indexOf(this.cur()) < 0) this.avancer(1);
    return { texte: this.t.slice(debut, this.p), ligne: ligne, colonne: colonne };
  }
}

/* ————————————————————— validation ————————————————————— */

function validerNombre(tok) {
  if (RE_NUM.test(tok.texte)) return;
  if (/^0[xX]/.test(tok.texte)) {
    throw erreur(tok.ligne, tok.colonne, "nombre hexadécimal sans chiffres : « " + tok.texte + " »");
  }
  throw erreur(tok.ligne, tok.colonne, "nombre invalide : « " + tok.texte + " »");
}

/* arg := NOMBRE | NOMBRE '-' NOMBRE | NOMBRE '-' | NOM (permission custom : file:read:file1_t). */
function validerArg(arg) {
  const t = arg.texte;
  if (RE_NUM.test(t) || RE_PLAGE.test(t)) return;
  if (RE_NOM.test(t)) return;
  if (t.indexOf(":") >= 0) {
    const parts = t.split(":");
    for (let i = 0; i < parts.length; i++) {
      if (!RE_NOM.test(parts[i])) {
        throw erreur(arg.ligne, arg.colonne, "argument d'ability mal formé : « " + t + " »");
      }
    }
    return;
  }
  if (/^0[xX]/.test(t)) {
    throw erreur(arg.ligne, arg.colonne, "nombre hexadécimal sans chiffres : « " + t + " »");
  }
  throw erreur(arg.ligne, arg.colonne, "argument d'ability mal formé : « " + t + " »");
}

/* ————————————————————— modèle ————————————————————— */

/* Une règle allow : src, dst optionnel, arête ou non (contrat §4). */
function ajouterRegle(etat, src, dst, ability, ligne, strict) {
  etat.regles.push({
    src: src.texte, srcPos: src,
    dst: dst ? dst.texte : null, dstPos: dst || null,
    ability: ability, ligne: ligne, strict: strict,
  });
}

/* Résolution finale : types déclarés -> arêtes, dans l'ordre source (contrat §4). */
function construireModele(etat) {
  const regles = [];
  etat.regles.forEach((r) => {
    if (!etat.declare.has(r.src)) {
      throw erreur(r.srcPos.ligne, r.srcPos.colonne, "type non déclaré : « " + r.src + " »");
    }
    if (r.dst === null) return;                       /* règle sans arête (bloc d'abilities) */
    if (r.strict && !etat.declare.has(r.dst)) {
      throw erreur(r.dstPos.ligne, r.dstPos.colonne, "type non déclaré : « " + r.dst + " »");
    }
    /* gain_priv vers un nom non déclaré : ability seule, aucune arête, aucune erreur (§4). */
    if (r.strict || etat.declare.has(r.dst)) {
      regles.push({ src: r.src, dst: r.dst, ability: r.ability, id: null, line: r.ligne });
    }
  });
  return {
    types: etat.types.slice(),
    rules: regles,
    untyped: [],
    hasDefaultSpawn: etat.hasDefaultSpawn,
  };
}

/* ————————————————————— productions ————————————————————— */

function parseType(lx, etat) {
  const nom = lx.lireIdent();
  if (!nom) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
  if (etat.declare.has(nom.texte)) {
    throw erreur(nom.ligne, nom.colonne, "type déjà déclaré : « " + nom.texte + " »");
  }
  etat.declare.add(nom.texte);
  etat.types.push(nom.texte);
  lx.attendre(";");
}

/* range IDENT ;  |  attribute IDENT ; */
function parseDeclSimple(lx) {
  if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom attendu");
  lx.attendre(";");
}

/* class IDENT { IDENT+ } ; */
function parseClass(lx) {
  if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de classe attendu");
  lx.attendre("{");
  for (;;) {
    lx.skipEspaces();
    if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
    if (lx.cur() === "}") { lx.avancer(1); break; }
    if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "permission attendue");
  }
  lx.attendre(";");
}

/* Un élément d'ability : modificateur, ou MOT [':' liste_args]. */
function parseAbilityItem(lx, etat, src, ligneRegle) {
  const mot = lx.lireBrut(": \t\r\n;}");
  if (mot.texte === "") throw erreur(lx.ligne, lx.colonne, "élément d'ability vide");
  const modificateur = MODIFICATEURS.indexOf(mot.texte) >= 0;
  if (!modificateur && !RE_MOT.test(mot.texte)) {
    throw erreur(mot.ligne, mot.colonne, "élément d'ability mal formé : « " + mot.texte + " »");
  }
  lx.skipEspaces();
  if (lx.cur() !== ":") return;                      /* ability sans argument */
  if (modificateur) {
    throw erreur(lx.ligne, lx.colonne, "modificateur sans argument : « " + mot.texte + " »");
  }
  lx.avancer(1);
  for (;;) {
    const arg = lx.lireBrut(", \t\r\n;}");
    if (arg.texte === "") throw erreur(lx.ligne, lx.colonne, "argument d'ability manquant");
    validerArg(arg);
    if (TRANSITIONS.indexOf(mot.texte) >= 0 && src) {
      ajouterRegle(etat, src, arg, mot.texte, ligneRegle, mot.texte === "settypeid");
    }
    if (lx.cur() === ",") { lx.avancer(1); continue; }
    return;
  }
}

/* bloc_ability : suite d'éléments jusqu'à '}' (src = null -> aucune arête). */
function parseBloc(lx, etat, src, ligneRegle) {
  for (;;) {
    lx.skipEspaces();
    if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
    if (lx.cur() === "}") { lx.avancer(1); return; }
    parseAbilityItem(lx, etat, src, ligneRegle);
  }
}

/* allow NOM NOM : channel connect ;  |  allow NOM NOM : ability { … } ; */
function parseAllow(lx, etat, ligneRegle) {
  const src = lx.lireIdent();
  if (!src) throw erreur(lx.ligne, lx.colonne, "type source attendu");
  const dst = lx.lireIdent();
  if (!dst) throw erreur(lx.ligne, lx.colonne, "type cible attendu");
  lx.attendre(":");
  const mot = lx.lireIdent();
  if (!mot) throw erreur(lx.ligne, lx.colonne, "mot-clé attendu après « : »");
  if (mot.texte === "channel") {
    const connect = lx.lireIdent();
    if (!connect || connect.texte !== "connect") {
      throw erreur(lx.ligne, lx.colonne, "« connect » attendu après « channel »");
    }
    lx.attendre(";");
    ajouterRegle(etat, src, dst, "channel", ligneRegle, true);
    return;
  }
  if (mot.texte === "ability") {
    if (dst.texte !== "self") {
      throw erreur(dst.ligne, dst.colonne, "une règle d'abilities doit cibler « self »");
    }
    ajouterRegle(etat, src, null, "ability", ligneRegle, false);
    lx.attendre("{");
    parseBloc(lx, etat, src, ligneRegle);
    lx.attendre(";");
    return;
  }
  throw erreur(mot.ligne, mot.colonne, "« ability » ou « channel » attendu");
}

/* allow_attach / allow_link : chemin (+ second chemin pour allow_attach) ou bloc. */
function parseChemin(lx, etat, deuxChemins) {
  if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "type attendu");
  lx.skipEspaces();
  if (lx.cur() === "{") {
    lx.avancer(1);
    parseBloc(lx, etat, null, lx.ligne);
    lx.attendre(";");
    return;
  }
  const p1 = lx.lireBrut(" \t\r\n;{}");
  if (p1.texte === "") throw erreur(lx.ligne, lx.colonne, "chemin attendu");
  lx.skipEspaces();
  if (!deuxChemins || lx.cur() === ";") { lx.attendre(";"); return; }
  const p2 = lx.lireBrut(" \t\r\n;{}");
  if (p2.texte === "") throw erreur(lx.ligne, lx.colonne, "chemin attendu");
  lx.attendre(";");
}

/* derive_type { NOM+ } { bloc } NOM ;  |  derive_type NOM NOM NOM ; */
function parseDeriveType(lx, etat) {
  lx.skipEspaces();
  if (lx.cur() !== "{") {
    if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
    if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
    if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
    lx.attendre(";");
    return;
  }
  lx.avancer(1);
  let n = 0;
  for (;;) {
    lx.skipEspaces();
    if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
    if (lx.cur() === "}") { lx.avancer(1); break; }
    if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
    n++;
  }
  if (n === 0) throw erreur(lx.ligne, lx.colonne, "liste de types vide");
  lx.attendre("{");
  parseBloc(lx, etat, null, lx.ligne);
  if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
  lx.attendre(";");
}

/* default_spawn_type NOM+ ; */
function parseDefaultSpawn(lx, etat) {
  if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
  lx.skipEspaces();
  while (lx.cur() !== ";") {
    if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "« ; » attendu");
    if (!lx.lireIdent()) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
    lx.skipEspaces();
  }
  lx.attendre(";");
  etat.hasDefaultSpawn = true;
}

/* mount_point NOMBRE : NOMBRE { bloc } ; */
function parseMountPoint(lx, etat) {
  const uid = lx.lireNombre();
  if (!uid) throw erreur(lx.ligne, lx.colonne, "uid attendu");
  validerNombre(uid);
  lx.attendre(":");
  const gid = lx.lireNombre();
  if (!gid) throw erreur(lx.ligne, lx.colonne, "gid attendu");
  validerNombre(gid);
  lx.attendre("{");
  parseBloc(lx, etat, null, lx.ligne);
  lx.attendre(";");
}

/* ————————————————————— pilote ————————————————————— */

function parseProgramme(lx, etat) {
  for (;;) {
    lx.skipEspaces();
    if (lx.fin()) return;
    const kw = lx.lireIdent();
    if (!kw) throw erreur(lx.ligne, lx.colonne, "mot-clé de production attendu");
    switch (kw.texte) {
      case "type": parseType(lx, etat); break;
      case "range": case "attribute": parseDeclSimple(lx); break;
      case "class": parseClass(lx); break;
      case "allow": parseAllow(lx, etat, kw.ligne); break;
      case "allow_attach": parseChemin(lx, etat, true); break;
      case "allow_link": parseChemin(lx, etat, false); break;
      case "derive_type": parseDeriveType(lx, etat); break;
      case "default_spawn_type": parseDefaultSpawn(lx, etat); break;
      case "mount_point": parseMountPoint(lx, etat); break;
      default:
        throw erreur(kw.ligne, kw.colonne, "mot-clé de production inconnu : « " + kw.texte + " »");
    }
  }
}

/* API publique : parseSecpol(texte) -> { model, error }. Jamais d'analyse partielle. */
function parseSecpol(texte) {
  try {
    const lx = new Lexer(texte);
    const etat = {
      types: [], declare: new Set(), regles: [], hasDefaultSpawn: false,
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
