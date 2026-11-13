"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — parseur du langage de politique de sécurité QNX 8.0 (secpol), côté navigateur.
 * Jumeau de core/src/secpol.c : mêmes productions, mêmes causes d'erreur, mêmes positions.
 * Contrat : docs/SECPOL-PARSEUR.md §2-§5 — grammaire gelée : docs/GRAMMAR.md.
 * DOM-free, sans dépendance : window.TamisSecpol dans le navigateur, module.exports sous Node.
 * Sous-ensemble STRICT : à la première erreur, { model: null, error: { line, column, message } }.
 *
 * Découpage (règle < 500 lignes/fichier) :
 *   tamis-secpol-abil.js  — vocabulaire fermé des abilities (jumeau de secpol_abil.c)
 *   tamis-secpol-decl.js  — tables de déclarations (jumeau de secpol_decl.c)
 *   tamis-secpol-lex.js   — lexique (jumeau de secpol_lex.c)
 *   tamis-secpol-prod.js  — productions + pilote + modèle (jumeau de secpol_parse.c/secpol.c)
 */

function modules() {
  if (typeof require === "function") {
    return {
      L: require("./tamis-secpol-lex.js"),
      P: require("./tamis-secpol-prod.js"),
    };
  }
  return { L: window.TamisSecpolLex, P: window.TamisSecpolProd };
}

/* API publique : parseSecpol(texte) -> { model, error }. Jamais d'analyse partielle. */
function parseSecpol(texte) {
  const { L, P } = modules();
  try {
    const lx = new L.Lexer(texte);
    const etat = P.nouvelEtat();
    P.analyser(lx, etat);
    return { model: P.construireModele(etat), error: null };
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