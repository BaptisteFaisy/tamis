"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — lexique du langage secpol (QNX 8.0), côté navigateur.
 * Jumeau de core/src/secpol_lex.c : mêmes jetons, mêmes positions, mêmes messages.
 * window.TamisSecpolLex dans le navigateur, module.exports sous Node. */

const T_EOL = "fin", T_IDENT = "ident", T_NUM = "nombre", T_HEX = "hexa", T_PUNCT = "ponct";

function erreur(ligne, colonne, message) {
  const e = new Error(message);
  e.tamisParse = true;
  e.ligne = ligne;
  e.colonne = colonne;
  return e;
}

/* Un caractère débute-t-il un identifiant ? (lettre ou souligné) */
function estDepartIdent(c) { return (c >= "a" && c <= "z") || (c >= "A" && c <= "Z") || c === "_"; }
function estIdent(c) { return estDepartIdent(c) || (c >= "0" && c <= "9"); }
function estChiffre(c) { return c >= "0" && c <= "9"; }
function estHexa(c) { return estChiffre(c) || (c >= "a" && c <= "f") || (c >= "A" && c <= "F"); }
function estBlanc(c) {
  return c === " " || c === "\t" || c === "\r" || c === "\n" || c === "\f" || c === "\v";
}

/* Lexique : balayage caractère par caractère (contrat docs/SECPOL-PARSEUR.md §2). */
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
      if (estBlanc(c)) { this.avancer(1); }
      else if (c === "#") { while (!this.fin() && this.cur() !== "\n") this.avancer(1); }
      else { return; }
    }
  }
  /* Jeton suivant : ident, nombre, ponctuation. -1 (via erreur) sur caractère inattendu. */
  suivant() {
    this.sauter();
    const ligne = this.ligne, colonne = this.colonne;
    if (this.fin()) return { kind: T_EOL, txt: "", ligne, colonne };
    const c = this.cur();
    if (estDepartIdent(c)) {
      const debut = this.p;
      while (!this.fin() && estIdent(this.cur())) this.avancer(1);
      return { kind: T_IDENT, txt: this.t.slice(debut, this.p), ligne, colonne };
    }
    if (estChiffre(c)) {
      const debut = this.p;
      const hexa = this.t[this.p] === "0" && (this.t[this.p + 1] === "x" || this.t[this.p + 1] === "X");
      if (hexa) { this.avancer(2); while (!this.fin() && estHexa(this.cur())) this.avancer(1); }
      else { while (!this.fin() && estChiffre(this.cur())) this.avancer(1); }
      return { kind: hexa ? T_HEX : T_NUM, txt: this.t.slice(debut, this.p), ligne, colonne };
    }
    if (c === ";" || c === "{" || c === "}" || c === ":") {
      this.avancer(1);
      return { kind: T_PUNCT, txt: c, ligne, colonne };
    }
    throw erreur(ligne, colonne, "caractère inattendu");
  }
  /* Mot brut jusqu'à un arrêtant : positions APRÈS consommation (comme lx_brut en C). */
  brut(arretants) {
    this.sauter();
    const debut = this.p;
    while (!this.fin() && arretants.indexOf(this.cur()) < 0) this.avancer(1);
    return { txt: this.t.slice(debut, this.p), ligne: this.ligne, colonne: this.colonne };
  }
  /* Bloc { … } imbriqué : le curseur est après l'accolade fermante. */
  sauterBloc() {
    let prof = 1;
    while (prof > 0) {
      if (this.fin()) throw erreur(this.ligne, this.colonne, "accolade fermante manquante");
      const c = this.cur();
      if (c === "{") { prof++; }
      else if (c === "}") { prof--; }
      this.avancer(1);
    }
  }
}

/* ————— aides du parseur (miroir de attendre_ident / attendre_pt / p_repere) ————— */

function attendreIdent(lx) {
  const j = lx.suivant();
  if (j.kind !== T_IDENT) throw erreur(j.ligne, j.colonne, "identifiant attendu");
  return j;
}

function attendrePt(lx, c) {
  const msg = c === ";" ? "« ; » attendu" : (c === "{" ? "« { » attendu" : "« } » attendu");
  let j;
  try {
    j = lx.suivant();
  } catch (e) {
    /* Comme le C : attendre_pt ignore le code de retour du lexique et rapporte
       la ponctuation attendue à la position du caractère fautif. */
    if (e && e.tamisParse) throw erreur(e.ligne, e.colonne, msg);
    throw e;
  }
  if (j.kind === T_EOL) throw erreur(j.ligne, j.colonne, "fin de fichier inattendue");
  if (j.kind !== T_PUNCT || j.txt !== c) throw erreur(j.ligne, j.colonne, msg);
}

/* Repère : un nom, ou un groupe { n1 n2 … }. Renvoie la liste des noms DANS L'ORDRE DU C
 * (le C empile en tête, donc l'ordre de liste est l'inverse de l'ordre source). */
function repere(lx) {
  lx.sauter();
  if (lx.cur() === "{") {
    lx.avancer(1);
    const noms = [];
    for (;;) {
      lx.sauter();
      if (lx.fin()) throw erreur(lx.ligne, lx.colonne, "accolade fermante manquante");
      if (lx.cur() === "}") { lx.avancer(1); return noms; }
      const mot = lx.brut(" \t\r\n;{}");
      if (mot.txt === "") throw erreur(lx.ligne, lx.colonne, "nom attendu dans le groupe");
      noms.unshift(mot.txt);                 /* empilement en tête, comme nom_lien_ajoute */
    }
  }
  const j = lx.suivant();
  if (j.kind !== T_IDENT) throw erreur(lx.ligne, lx.colonne, "nom de type attendu");
  return [j.txt];
}

/* Vrai si le repère est exactement « self ». */
function repereEstSelf(noms) { return noms.length === 1 && noms[0] === "self"; }

const TamisSecpolLex = { Lexer, erreur, attendreIdent, attendrePt, repere, repereEstSelf };
if (typeof module !== "undefined" && module.exports) module.exports = TamisSecpolLex;
if (typeof window !== "undefined") window.TamisSecpolLex = TamisSecpolLex;