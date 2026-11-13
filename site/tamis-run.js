"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — exécution partagée côté navigateur : détection de format, parse, commandes, rendu.
 * Un seul endroit qui sait « lire une politique et lancer une commande » : le playground
 * (app.js), le banc réel (bridge.js) et le codelab (codelab.js) s'appuient dessus — zéro
 * duplication de la logique de format ou de dispatch.
 * window.TamisRun dans le navigateur, module.exports sous Node. */

function deps() {
  if (typeof require === "function") {
    return {
      C: require("./tamis-core.js"),
      R: require("./tamis-render.js"),
      S: require("./tamis-secpol.js"),
    };
  }
  return { C: window.TamisCore, R: window.TamisRender, S: window.TamisSecpol };
}
const { C, R, S } = deps();

/* Vrai langage secpol s'il y a un « ; » hors commentaire ; sinon format provisoire .mpf. */
function detectFormat(texte) {
  const sans = String(texte || "").replace(/#[^\n]*/g, "");
  return sans.indexOf(";") >= 0 ? "secpol" : "mpf";
}

/* Parse dans le format détecté ; lève { line, column, message } en cas d'erreur. */
function parsePolicy(texte) {
  if (detectFormat(texte) === "secpol" && S) {
    const r = S.parseSecpol(texte);
    if (r.error) throw r.error;
    return r.model;
  }
  return C.parseModel(texte);
}

/* Verdict TAMIS_PARSE_ERROR à partir d'une erreur de parse. */
function verdictErreur(e, file) {
  return [{ code: 1, sev: 2, file, line: (e && e.line) || 0, column: (e && e.column) || 0,
    message: (e && e.message) || "erreur de syntaxe", src: null, dst: null, path: null, rules: null }];
}

/* Graphique du graphe de privilèges, même JSON que `build/tamis view` (tools/tamis.c). */
function vue(m, src, dst) {
  const edges = m.rules.map((r) => ({
    src: m.types[r.src], dst: m.types[r.dst], rule: r.id ? r.id : (r.ability || ""),
  }));
  const o = { nodes: m.types.slice(), edges };
  if (src && dst) o.focus = { src, dst };
  return JSON.stringify(o);
}

/* Résultat d'un moteur → verdicts + modèles (pour le graphe de l'appelant). */
function finaliser(res, mA, mB, label, src, dst) {
  if (res.err) {
    return { verdicts: [{ code: 12, sev: 2, file: label, line: 0, column: 0,
      message: "type inconnu : src ou dst absent du modèle", src: src || null, dst: dst || null,
      path: null, rules: null }], model: mA, modelB: mB, errType: true };
  }
  return { verdicts: res.verdicts, model: mA, modelB: mB };
}

/* Exécute une commande. opts = { cmd, policy, policy2, src, dst, label, label2 }.
 * Renvoie { verdicts, model, modelB } ou { vue } ou { nonComparable } ; jamais d'exception. */
function executer(o) {
  const label = o.label || "policy";
  const label2 = o.label2 || "candidate.mpf";
  let mA;
  try { mA = parsePolicy(o.policy); }
  catch (e) { return { verdicts: verdictErreur(e, label), phase: "policy" }; }

  if (o.cmd === "view") return { vue: vue(mA, o.src, o.dst), model: mA };

  try {
    if (o.cmd === "lint") return finaliser(C.lint(mA, label), mA, null, label);
    if (o.cmd === "minimize") return finaliser(C.minimize(mA, label), mA, null, label);
    if (o.cmd === "reach") return finaliser(C.reach(mA, o.src, o.dst, label), mA, null, label, o.src, o.dst);
    if (o.cmd === "proof") return finaliser(C.proof(mA, o.src, o.dst, label), mA, null, label, o.src, o.dst);
    if (o.cmd === "explain") return finaliser(C.explain(mA, o.src, o.dst, label), mA, null, label, o.src, o.dst);
    if (o.cmd === "diff") {
      let mB;
      try { mB = parsePolicy(o.policy2 || ""); }
      catch (e) { return { verdicts: verdictErreur(e, label2), phase: "policy2" }; }
      return finaliser(C.diff(mA, mB, label), mA, mB, label);
    }
    return { nonComparable: true, model: mA };
  } catch (e) {
    return { verdicts: verdictErreur(e, label), model: mA };
  }
}

/* Rendu d'une liste de verdicts dans le format demandé (text | json | md). */
function rendre(fmt, verdicts, meta) {
  const vs = verdicts || [];
  if (fmt === "json") return R.renderJson(vs, C.verdictName, C.sevName) || "(aucun verdict)";
  if (fmt === "md") {
    return R.renderMarkdown(vs, meta || {}, C.verdictName, C.sevName, C.DEFAULT_SEV, C.VERDICT_COUNT);
  }
  return vs.length ? R.renderText(vs, C.verdictName, C.sevName, C.SEV_LABEL) : "OK\n";
}

const TamisRun = { detectFormat, parsePolicy, verdictErreur, vue, executer, rendre };
if (typeof module !== "undefined" && module.exports) module.exports = TamisRun;
if (typeof window !== "undefined") window.TamisRun = TamisRun;