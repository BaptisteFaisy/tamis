"use strict";
/* SPDX-License-Identifier: MIT */
/* Smoke test du site : exécute site/app.js contre un mini-DOM (sans navigateur) et */
/* vérifie le câblage — démarrage, chaque commande, chaque format, le graphe, les onglets. */
const assert = require("assert");
const path = require("path");

let checks = 0;
function ck(cond, msg) {
  checks++;
  if (!cond) { console.log("FAILED : " + msg); process.exitCode = 1; }
}

/* ——— mini-DOM ——— */
function makeEl(id, tag) {
  const ev = {};
  return {
    id: id || "", tagName: (tag || "div").toUpperCase(), children: [],
    innerHTML: "", textContent: "", value: "", hidden: false, dataset: {},
    className: "", style: {}, onclick: null,
    classList: {
      _s: new Set(),
      add(c) { this._s.add(c); },
      remove(c) { this._s.delete(c); },
      contains(c) { return this._s.has(c); },
      toggle(c, f) {
        if (f === undefined) f = !this._s.has(c);
        if (f) this._s.add(c); else this._s.delete(c);
        return f;
      },
    },
    appendChild(c) { this.children.push(c); return c; },
    addEventListener(t, fn) { ev[t] = fn; },
    fire(t) { if (ev[t]) ev[t](); },
    setAttribute(k, v) { this[k] = v; },
    querySelectorAll() { return []; },
  };
}

const byId = {};
const TABS = ["playground", "features", "proofs", "submission", "codelab", "ideas", "grammar"];
const CMDS = ["lint", "reach", "proof", "explain", "minimize", "diff", "view"];
const FMTS = ["text", "json", "md"];
const LANGS = ["fr", "en"];

TABS.forEach((t) => { byId[t] = makeEl(t, "section"); });
["policyA", "policyB"].forEach((t) => { byId[t] = makeEl(t, "textarea"); });
["src", "dst", "example"].forEach((t) => { byId[t] = makeEl(t, "select"); });
["output", "graph", "graphStatus", "pairHint", "exitBadge", "panelB", "copy", "loadExample", "langNote"]
  .forEach((t) => { byId[t] = makeEl(t); });

const navBtns = TABS.map((t) => { const b = makeEl("", "button"); b.dataset.tab = t; return b; });
const toolBtns = CMDS.map((c) => { const b = makeEl("", "button"); b.dataset.cmd = c; return b; });
const fmtBtns = FMTS.map((f) => { const b = makeEl("", "button"); b.dataset.fmt = f; return b; });
const langBtns = LANGS.map((l) => { const b = makeEl("", "button"); b.dataset.langSet = l; return b; });

const store = {};
global.localStorage = {
  getItem: (k) => (k in store ? store[k] : null),
  setItem: (k, v) => { store[k] = String(v); },
};

global.document = {
  body: makeEl("", "body"),
  documentElement: makeEl("", "html"),
  getElementById: (id) => byId[id] || (byId[id] = makeEl(id)),
  createElement: (t) => makeEl("", t),
  createElementNS: (ns, t) => makeEl("", t),
  querySelectorAll: (sel) => {
    if (sel === "#nav button") return navBtns;
    if (sel === "#toolbar button") return toolBtns;
    if (sel === "#fmt button") return fmtBtns;
    if (sel === "#lang button") return langBtns;
    if (sel === ".tab") return TABS.map((t) => byId[t]);
    return [];
  },
};
global.window = global;
try {
  Object.defineProperty(global, "navigator", {
    value: { clipboard: { writeText: () => Promise.resolve() } },
    configurable: true,
  });
} catch (e) { /* Node fournit déjà un navigator (lecture seule) — inutilisé ici */ }

/* ——— chargement des modules du site ——— */
require(path.join(__dirname, "..", "site", "tamis-core.js"));
require(path.join(__dirname, "..", "site", "tamis-render.js"));
ck(typeof global.TamisCore === "object", "TamisCore exposé sur window");
ck(typeof global.TamisRender === "object", "TamisRender exposé sur window");

require(path.join(__dirname, "..", "site", "app.js"));

/* ——— démarrage : lint sur la fixture « attaque » ——— */
const out = byId.output;
ck(out.textContent.length > 0, "une sortie est produite au démarrage");
ck(out.textContent.indexOf("TAMIS_UNTYPED_PROCESS") >= 0, "lint au démarrage : UNTYPED_PROCESS");
ck(out.textContent.indexOf("TAMIS_MISSING_DEFAULT_SPAWN") >= 0, "lint au démarrage : MISSING_DEFAULT_SPAWN");
ck(byId.src.value === "app_tiers", "src pré-rempli (premier type)");
ck(byId.dst.value === "reseau", "dst pré-rempli (dernier type)");
ck(byId.graph.children.length > 0, "graphe dessiné au démarrage");
ck(byId.exitBadge.textContent.indexOf("exit 0") === 0, "badge exit 0 (warnings seuls)");

/* ——— chaque commande doit produire une sortie ——— */
const attendu = {
  lint: "UNTYPED_PROCESS", reach: "", proof: "", explain: "",
  minimize: "REDUNDANT_RULE", diff: "", view: "",
};
toolBtns.forEach((b) => {
  byId.src.value = "app_tiers";
  byId.dst.value = "pompe_critique";
  b.onclick();
  ck(typeof out.textContent === "string", "commande " + b.dataset.cmd + " : sortie produite");
  if (attendu[b.dataset.cmd]) {
    ck(out.textContent.indexOf(attendu[b.dataset.cmd]) >= 0,
      "commande " + b.dataset.cmd + " : contient " + attendu[b.dataset.cmd]);
  }
});

/* reach sur attaque doit trouver le chemin d'attaque */
toolBtns.find((b) => b.dataset.cmd === "reach").onclick();
ck(out.textContent.indexOf("TAMIS_PATH_FOUND") >= 0, "reach trouve le chemin");
ck(out.textContent.indexOf("app_tiers -> pompe_critique") >= 0, "reach affiche le chemin");
ck(byId.exitBadge.textContent.indexOf("exit 1") === 0, "badge exit 1 sur error");
ck(/chemin|path/.test(byId.graphStatus.textContent), "statut du graphe : chemin signalé");

/* proof sur la politique durcie doit donner le certificat */
byId.policyA.value = byId.policyB.value;
byId.example.value = "durcie";
byId.loadExample.onclick();
toolBtns.find((b) => b.dataset.cmd === "proof").onclick();
ck(out.textContent.indexOf("non-interférence prouvée") >= 0, "proof : certificat sur politique durcie");
ck(byId.exitBadge.textContent.indexOf("exit 0") === 0, "badge exit 0 après correctif");

/* ——— les trois formats ——— */
byId.example.value = "attaque";
byId.loadExample.onclick();
toolBtns.find((b) => b.dataset.cmd === "lint").onclick();
fmtBtns.find((b) => b.dataset.fmt === "json").onclick();
ck(out.textContent.trim().startsWith("{"), "format JSON : objet en sortie");
ck(out.textContent.indexOf('"code":"TAMIS_UNTYPED_PROCESS"') >= 0, "format JSON : clé code");
fmtBtns.find((b) => b.dataset.fmt === "md").onclick();
ck(out.textContent.indexOf("# Rapport d'audit Tamis") >= 0, "format Markdown : titre du rapport");
ck(out.textContent.indexOf("## Synthèse") >= 0, "format Markdown : synthèse");
fmtBtns.find((b) => b.dataset.fmt === "text").onclick();
ck(out.textContent.indexOf("WARN   TAMIS_UNTYPED_PROCESS") >= 0, "format texte : préfixe aligné");

/* ——— panneau diff visible uniquement pour diff ——— */
toolBtns.find((b) => b.dataset.cmd === "diff").onclick();
ck(byId.panelB.hidden === false, "diff : le panneau candidat est visible");
toolBtns.find((b) => b.dataset.cmd === "lint").onclick();
ck(byId.panelB.hidden === true, "lint : le panneau candidat est masqué");

/* ——— onglets ——— */
navBtns.find((b) => b.dataset.tab === "ideas").onclick();
ck(byId.ideas.classList.contains("active"), "onglet idées activé");
ck(!byId.playground.classList.contains("active"), "onglet playground désactivé");

/* ——— bascule de langue FR/EN ——— */
byId.example.value = "attaque";
byId.loadExample.onclick();
byId.src.value = "app_tiers";
byId.dst.value = "pompe_critique";
toolBtns.find((b) => b.dataset.cmd === "reach").onclick();
const statusEn = byId.graphStatus.textContent;
ck(statusEn.indexOf("path") >= 0, "EN : statut du graphe en anglais");

langBtns.find((b) => b.dataset.langSet === "fr").onclick();
ck(document.body.classList.contains("lang-fr"), "FR : classe lang-fr sur body");
ck(!document.body.classList.contains("lang-en"), "FR : lang-en retirée");
ck(document.documentElement.lang === "fr", "FR : attribut lang=fr sur <html>");
ck(byId.langNote.textContent.indexOf("cœur C") >= 0, "FR : note sur la langue des messages");
ck(byId.graphStatus.textContent.indexOf("chemin") >= 0, "FR : statut du graphe traduit");
ck(byId.graphStatus.textContent !== statusEn, "le statut change bien avec la langue");
ck(store["tamis-lang"] === "fr", "choix de langue persisté");

langBtns.find((b) => b.dataset.langSet === "en").onclick();
ck(document.body.classList.contains("lang-en"), "EN : retour à lang-en");
ck(byId.langNote.textContent.indexOf("C core") >= 0, "EN : note sur la langue des messages");
ck(byId.output.textContent.indexOf("TAMIS_PATH_FOUND") >= 0, "la sortie survit au changement de langue");

/* ——— erreur de syntaxe remontée proprement ——— */
byId.policyA.value = "ceci nest pas une politique\n";
byId.policyA.fire("change");
toolBtns.find((b) => b.dataset.cmd === "lint").onclick();
ck(out.textContent.indexOf("TAMIS_PARSE_ERROR") >= 0, "politique invalide : TAMIS_PARSE_ERROR affiché");
ck(byId.exitBadge.textContent.indexOf("exit 1") === 0, "politique invalide : exit 1");

console.log(process.exitCode ? "FAILED test_site" : "OK " + checks + " checks");
