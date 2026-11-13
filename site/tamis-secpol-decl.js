"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — tables de déclarations du parseur secpol, côté navigateur.
 * Jumeau de core/src/secpol_decl.c : types, classes, permissions, attributs,
 * abilities dynamiques, plages nommées, relabel de canal.
 * window.TamisSecpolDecl dans le navigateur, module.exports sous Node. */

/* Erreur de parse : même contrat que le C (line, column, message). */
function erreur(ligne, colonne, message) {
  const e = new Error(message);
  e.tamisParse = true;
  e.ligne = ligne;
  e.colonne = colonne;
  return e;
}

/* ————— plage numérique : miroir de plage_valide() (core/src/secpol.c) —————
 * 1 = valide, 0 = mal formée, 2 = fin avant le début. */
function plageValide(plage) {
  if (!plage || plage.length === 0) return 0;
  let p = plage, debut = 0, fin = 0, chiffres = false;
  const estCh = (c) => c >= "0" && c <= "9";
  const estHx = (c) => estCh(c) || (c >= "a" && c <= "f") || (c >= "A" && c <= "F");
  const val = (c) => (estCh(c) ? c.charCodeAt(0) - 48 : c.toLowerCase().charCodeAt(0) - 87);
  if (p[0] === "0" && (p[1] === "x" || p[1] === "X")) {
    p = p.slice(2);
    if (!estHx(p[0] || "")) return 0;
    while (estHx(p[0] || "")) { debut = debut * 16 + val(p[0]); chiffres = true; p = p.slice(1); }
  } else {
    while (estCh(p[0] || "")) { debut = debut * 10 + (p.charCodeAt(0) - 48); chiffres = true; p = p.slice(1); }
  }
  if (!chiffres) return 0;
  if (p.length === 0) return 1;
  if (p[0] !== "-") return 0;
  p = p.slice(1);
  if (p.length === 0) return 1;               /* borne ouverte « 96- » */
  if (p[0] === "0" && (p[1] === "x" || p[1] === "X")) {
    p = p.slice(2);
    if (!estHx(p[0] || "")) return 0;
    while (estHx(p[0] || "")) { fin = fin * 16 + val(p[0]); p = p.slice(1); }
  } else {
    let vu = false;
    while (estCh(p[0] || "")) { fin = fin * 10 + (p.charCodeAt(0) - 48); vu = true; p = p.slice(1); }
    if (fin === 0 && !vu) return 0;
  }
  if (p.length !== 0) return 0;               /* « 4-6-9 » : reste des caractères */
  return fin < debut ? 2 : 1;
}

/* ————— tables et prédicats ————— */
function typeDeclare(etat, nom) { return etat.declare.has(nom); }
function attributDeclare(etat, nom) { return etat.attributs.has(nom); }
function porteeAttribut(etat, nom) { return typeDeclare(etat, nom) || attributDeclare(etat, nom); }
function abilityDeclaree(etat, nom) { return etat.abilites.has(nom); }
function rangeDeclaree(etat, nom) { return etat.ranges.has(nom); }
function classeDeclaree(etat, nom) { return etat.classes.has(nom); }
function permissionDeclaree(etat, classe, perm) {
  const c = etat.classes.get(classe);
  return c ? c.has(perm) : false;
}

/* Résout un nom en liste de types : [terme] si type, membres si attribut, null sinon. */
function resoudreTerme(etat, terme) {
  if (typeDeclare(etat, terme)) return [terme];
  const m = etat.membres.get(terme);
  return m && m.length ? m.slice() : null;
}

/* Déclarations avec contrôles de doublon (messages identiques au C). */
function declarerAttribut(etat, nom, ligne, colonne) {
  if (attributDeclare(etat, nom)) throw erreur(ligne, colonne, "attribut déjà déclaré");
  etat.attributs.add(nom);
}
function declarerRange(etat, nom) {          /* re-déclaration acceptée par le compilateur */
  etat.ranges.add(nom);
}
function declarerAbility(etat, nom, ligne, colonne) {
  if (abilityDeclaree(etat, nom)) throw erreur(ligne, colonne, "ability déjà déclarée");
  etat.abilites.add(nom);
}
function declarerClasse(etat, nom, ligne, colonne) {
  if (classeDeclaree(etat, nom)) throw erreur(ligne, colonne, "classe déjà déclarée");
  etat.classes.set(nom, new Set());
}
function declarerPermission(etat, classe, perm, ligne, colonne) {
  const c = etat.classes.get(classe);
  if (c && c.has(perm)) {
    throw erreur(ligne, colonne, "permission déjà déclarée pour cette classe");
  }
  c.add(perm);
}
function declarerMembre(etat, attr, membre) {
  if (!etat.membres.has(attr)) etat.membres.set(attr, []);
  etat.membres.get(attr).push(membre);
}

/*
 * Relabel de canal : toutes les règles d'un même type relabelent vers le MÊME type,
 * ou aucune ne le fait (contrôle du compilateur officiel). */
function declarerRelabel(etat, type, cible, ligne, colonne) {
  const deja = etat.relabels.get(type) || [];
  for (const r of deja) {
    const memeNull = (r === null) === (cible === null);
    if (memeNull && (cible === null || r === cible)) return;   /* cohérent avec le passé */
    throw erreur(ligne, colonne,
      "relabel de canal incohérent : toutes les règles de ce type doivent relabeler vers le même type");
  }
  deja.push(cible);
  etat.relabels.set(type, deja);
}

const TamisSecpolDecl = {
  erreur, plageValide,
  typeDeclare, attributDeclare, porteeAttribut, abilityDeclaree,
  rangeDeclaree, classeDeclaree, permissionDeclaree, resoudreTerme,
  declarerAttribut, declarerRange, declarerAbility, declarerClasse, declarerPermission,
  declarerMembre, declarerRelabel,
};
if (typeof module !== "undefined" && module.exports) module.exports = TamisSecpolDecl;
if (typeof window !== "undefined") window.TamisSecpolDecl = TamisSecpolDecl;