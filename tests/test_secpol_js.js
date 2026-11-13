"use strict";
/* SPDX-License-Identifier: MIT */
/* Test du parseur secpol côté navigateur (site/tamis-secpol.js).
 * Autonome : `node tests/test_secpol_js.js` — ≥ 40 vérifications, sortie « OK n checks ».
 * Contrat vérifié : docs/SECPOL-PARSEUR.md §2-§5 (lexique, productions, modèle, erreurs). */

const fs = require("fs");
const path = require("path");

const MODULE = path.join(__dirname, "..", "site", "tamis-secpol.js");
const { parseSecpol } = require(MODULE);

let checks = 0;
function ck(cond, msg) {
  checks++;
  if (!cond) { console.log("FAILED : " + msg); process.exitCode = 1; }
}

/* ————————————————————— helpers ————————————————————— */

/* Analyse et vérifie l'absence d'erreur ; renvoie le modèle. */
function ok(texte, msg) {
  const r = parseSecpol(texte);
  ck(r.error === null, msg + " : aucune erreur (" + (r.error ? r.error.message : "") + ")");
  ck(r.model !== null, msg + " : modèle non nul");
  return r.model;
}

/* Analyse et vérifie le refus ; renvoie l'erreur (ligne non nulle, colonne ≥ 0).
 * La colonne peut être 0 quand le C lui-même n'en donne pas (ex. « cible non-self »). */
function ko(texte, msg) {
  const r = parseSecpol(texte);
  ck(r.error !== null, msg + " : erreur attendue");
  ck(r.model === null, msg + " : modèle nul en cas d'erreur (pas d'analyse partielle)");
  const e = r.error || { line: 0, column: 0, message: "" };
  ck(typeof e.line === "number" && e.line > 0, msg + " : ligne non nulle (" + e.line + ")");
  ck(typeof e.column === "number" && e.column >= 0, msg + " : colonne numérique (" + e.column + ")");
  ck(typeof e.message === "string" && e.message.length > 0, msg + " : cause non vide");
  return e;
}

/* Le modèle suit le contrat du parseur C : `types` (noms), `idx` (nom -> indice),
 * et `rules[].src/dst` sont des INDICES dans `types` (comme tamis_model). */
function types(model) { return model.types; }
function regles(model) { return model.rules; }
function arete(model, src, dst) {
  const s = model.idx[src], d = model.idx[dst];
  return model.rules.filter((r) => r.src === s && r.dst === d);
}

/* ————————————————————— 1. politique minimale ————————————————————— */

{
  const m = ok("type app_tiers;\ntype journal;\n"
             + "allow app_tiers self:ability { settypeid:journal };\n", "politique minimale");
  ck(types(m).length === 2, "2 types déclarés");
  ck(types(m)[0] === "app_tiers" && types(m)[1] === "journal", "noms de types dans l'ordre source");
  ck(regles(m).length === 1, "1 arête");
  ck(regles(m)[0].src === m.idx["app_tiers"], "arête : source app_tiers (indice)");
  ck(regles(m)[0].dst === m.idx["journal"], "arête : cible journal (indice)");
  ck(regles(m)[0].ability === "settypeid", "arête : libellé settypeid");
  ck(regles(m)[0].line === 3, "arête : ligne de la règle allow");
  ck(regles(m)[0].id === null, "arête : identifiant de règle nul");
  ck(m.untyped.length === 0, "aucun processus non typé");
  ck(m.hasDefaultSpawn === false, "pas de default_spawn_type");
}

/* ————————————————————— 2. les 8 formes de production (§3) ————————————————————— */

/* (a) allow NOM NOM : channel connect ; */
{
  const m = ok("type a;\ntype b;\nallow a b:channel connect;\n", "forme (a) canal");
  ck(regles(m).length === 1, "(a) : 1 arête");
  ck(regles(m)[0].ability === "channel", "(a) : libellé channel");
  ck(regles(m)[0].src === m.idx["a"] && regles(m)[0].dst === m.idx["b"], "(a) : a -> b");
}
/* (b) allow NOM NOM : ability { bloc } ; */
{
  const m = ok("type a;\ntype b;\nallow a self:ability { settypeid:b };\n", "forme (b) abilities");
  ck(regles(m).length === 1, "(b) : 1 arête");
  ck(regles(m)[0].ability === "settypeid", "(b) : libellé settypeid");
}
/* (c) allow_attach IDENT CHEMIN [ CHEMIN ] ; */
{
  const m = ok("type a;\ntype b;\nallow_attach a /dev/socket/* b;\n", "forme (c) attachage");
  ck(regles(m).length === 0, "(c) : aucune arête");
  const m2 = ok("type a;\nallow_attach a /dev/random;\n", "forme (c) attachage 1 argument");
  ck(m2 !== null, "(c) : 1 argument accepté");
}
/* (d) allow_attach IDENT { bloc } ; */
{
  const m = ok("type a;\nallow_attach a { /dev/random };\n", "forme (d) attachage bloc");
  ck(regles(m).length === 0, "(d) : aucune arête");
}
/* (e) allow_link IDENT CHEMIN ; */
{
  const m = ok("type a;\nallow_link a /usr/lib/ldqnx-64.so.2;\n", "forme (e) allow_link");
  ck(regles(m).length === 0, "(e) : aucune arête");
}
/* (f) allow_link IDENT { bloc } ; */
{
  const m = ok("type a;\nallow_link a { /usr/lib/ldqnx-64.so.2 };\n", "forme (f) allow_link bloc");
  ck(regles(m).length === 0, "(f) : aucune arête");
}
/* (g) derive_type NOM NOM NOM ; */
{
  const m = ok("type a;\ntype b;\nderive_type a name b;\n", "forme (g) derive_type");
  ck(regles(m).length === 0, "(g) : aucune arête");
}
/* (h) derive_type { NOM+ } { bloc } NOM ; */
{
  const m = ok("type a;\ntype b;\ntype c;\nderive_type { a b } { settypeid:c } c;\n",
               "forme (h) derive_type groupé");
  ck(regles(m).length === 0, "(h) : aucune arête");
}

/* ————————————————————— 3. arêtes : settypeid / gain_priv / canal ————————————————————— */

{
  const m = ok("type a;\ntype b;\ntype c;\n"
             + "allow a self:ability { settypeid:b };\n"
             + "allow a self:ability { gain_priv:c };\n"
             + "allow a c:channel connect;\n", "arêtes des trois sources");
  ck(regles(m).length === 3, "3 arêtes");
  ck(arete(m, "a", "b").length === 1, "arête settypeid : a -> b");
  ck(arete(m, "a", "c").length === 2, "arête gain_priv + arête canal vers c");
  ck(arete(m, "a", "b")[0].ability === "settypeid", "libellé settypeid");
  ck(arete(m, "a", "c").some((r) => r.ability === "gain_priv"), "libellé gain_priv");
  ck(arete(m, "a", "c").some((r) => r.ability === "channel"), "libellé channel");
  /* gain_priv vers un nom d'ability (non un type) : aucune arête, aucune erreur */
  const m2 = ok("type a;\nallow a self:ability { gain_priv:pathspace };\n",
                "gain_priv vers une ability");
  ck(regles(m2).length === 0, "gain_priv vers une ability : aucune arête");
}

/* ————————————————————— 4. default_spawn_type ————————————————————— */

{
  const m = ok("type a;\ntype b;\ndefault_spawn_type a b;\n", "default_spawn_type");
  ck(m.hasDefaultSpawn === true, "has_default_spawn positionné");
  ck(regles(m).length === 0, "default_spawn_type : aucune arête");
  const m2 = ok("type a;\ntype b;\nallow a self:ability { settypeid:b };\n",
                "politique sans default_spawn_type");
  ck(m2.hasDefaultSpawn === false, "has_default_spawn faux par défaut");
}

/* ————————————————————— 5. commentaires ————————————————————— */

{
  const m = ok("# commentaire de tête\ntype a; # commentaire en fin de ligne\n"
             + "type b;\nallow a self:ability {\n  # commentaire interne\n  settypeid:b\n};\n",
             "commentaires");
  ck(types(m).length === 2, "commentaires : 2 types");
  ck(regles(m).length === 1, "commentaires : 1 arête");
  ck(regles(m)[0].line === 4, "commentaires : ligne de la règle allow");
}

/* ————————————————————— 6. plages : hexadécimales et bornes ouvertes ————————————————————— */

{
  const m = ok("type a;\nallow a self:ability {\n"
             + "  mem_phys:1024-4096,0x1200000-0x24000000\n"
             + "  setuid:4-6,23,96-\n};\n", "plages hexadécimales et bornes ouvertes");
  ck(regles(m).length === 0, "plages : aucune arête");
  const m2 = ok("type a;\ntype b;\nallow a self:ability { setuid:96- };\n", "borne ouverte isolée");
  ck(m2 !== null, "borne ouverte « 96- » acceptée");
}

/* ————————————————————— 7. ability dynamique ————————————————————— */

{
  const m = ok("ability network/bind/privport;\ntype a;\n"
             + "allow a self:ability { network/bind/privport };\n", "ability dynamique");
  ck(types(m).length === 1, "ability dynamique : 1 type");
  ck(regles(m).length === 0, "ability dynamique : aucune arête");
}

/* ————————————————————— 8. classe custom ————————————————————— */

{
  const m = ok("class file { read write delete execute };\ntype a;\ntype f1;\n"
             + "allow a self:ability { gain_priv: file:read:f1 };\n", "classe custom");
  ck(types(m).length === 2, "classe custom : 2 types");
  ck(regles(m).length === 0, "classe custom : aucune arête");
}

/* ————————————————————— 9. erreurs ————————————————————— */

/* « ; » manquant */
{
  const e = ko("type a\n", "point-virgule manquant");
  ck(e.line === 2, "point-virgule manquant : ligne");
}
/* « } » manquant */
{
  const e = ko("type a;\nallow a self:ability { able_create\n", "accolade fermante manquante");
  ck(e.line === 3, "accolade fermante manquante : ligne");
}
/* type non déclaré (cible d'une transition) */
{
  const e = ko("type a;\nallow a self:ability { settypeid:fantome };\n", "type cible non déclaré");
  ck(e.message.indexOf("non déclaré") >= 0, "type cible non déclaré : cause");
}
/* type non déclaré (source d'une règle) */
{
  const e = ko("allow x self:ability { settypeid:y };\n", "type source non déclaré");
  ck(e.message.indexOf("non déclaré") >= 0, "type source non déclaré : cause");
}
/* type dupliqué */
{
  const e = ko("type a;\ntype a;\n", "type dupliqué");
  ck(e.message.indexOf("déjà déclaré") >= 0, "type dupliqué : cause");
  ck(e.line === 2, "type dupliqué : ligne de la 2e déclaration");
}
/* ability_item mal formé : argument absent */
{
  const e = ko("type a;\nallow a self:ability { setuid: };\n", "ability_item sans argument");
  ck(e.message.indexOf("argument") >= 0, "ability_item sans argument : cause");
}
/* ability_item mal formé : plage « - » seule — le C la traite comme un nom de plage */
{
  const e = ko("type a;\nallow a self:ability { setuid:- };\n", "ability_item plage « - » seule");
  ck(e.message.indexOf("plage nommée non déclarée") >= 0, "ability_item plage « - » seule : cause");
}
/* ability_item mal formé : « 0x » sans chiffres */
{
  const e = ko("type a;\nallow a self:ability { mem_phys:0x };\n", "ability_item « 0x » sans chiffres");
  ck(e.message.indexOf("plage mal formée") >= 0, "ability_item « 0x » : cause");
}
/* cible d'abilities non-self */
{
  const e = ko("type a;\ntype b;\nallow a b:ability { able_create };\n", "cible non-self");
  ck(e.message.indexOf("self") >= 0, "cible non-self : cause");
}
/* mot-clé de production inconnu */
{
  const e = ko("toto x;\n", "mot-clé inconnu");
  ck(e.message.indexOf("inconnu") >= 0, "mot-clé inconnu : cause");
  ck(e.line === 1, "mot-clé inconnu : ligne");
}

/* ————————————————————— 10. la politique QNX 8.0 réelle ————————————————————— */

{
  const f = path.join(__dirname, "..", "fixtures", "politique-qnx8.secpol");
  if (fs.existsSync(f)) {
    const m = ok(fs.readFileSync(f, "utf8"), "politique QNX 8.0 officielle");
    ck(types(m).length === 9, "politique QNX 8.0 : 9 types");
    ck(regles(m).length === 4, "politique QNX 8.0 : 4 arêtes");
    ck(arete(m, "app_tiers", "journal").length === 1, "politique QNX 8.0 : app_tiers -> journal");
    ck(arete(m, "journal", "diag").length === 1, "politique QNX 8.0 : journal -> diag");
    ck(arete(m, "diag", "pompe_critique").length === 1, "politique QNX 8.0 : diag -> pompe_critique");
    ck(arete(m, "mm_client", "mm_server").length === 1, "politique QNX 8.0 : mm_client -> mm_server");
    ck(arete(m, "app_tiers", "file1_t").length === 0,
       "politique QNX 8.0 : gain_priv custom sans arête");
  }
}

/* ————————————————————— 11. API et export ————————————————————— */

{
  const r = parseSecpol("type a;\n");
  ck(typeof r === "object", "parseSecpol renvoie un objet");
  ck(Object.prototype.hasOwnProperty.call(r, "model"), "résultat : clé model");
  ck(Object.prototype.hasOwnProperty.call(r, "error"), "résultat : clé error");
  ck(r.error === null, "résultat : error nul en cas de succès");
  ck(typeof parseSecpol === "function", "parseSecpol exportée");
  /* Export navigateur : window.TamisSecpol (comme tamis-core.js). */
  const cle = require.resolve(MODULE);
  const cache = require.cache[cle];
  const windowAvant = global.window;
  global.window = {};
  delete require.cache[cle];
  require(MODULE);
  ck(typeof global.window.TamisSecpol === "object", "window.TamisSecpol exposé");
  ck(typeof global.window.TamisSecpol.parseSecpol === "function", "window.TamisSecpol.parseSecpol");
  if (cache) require.cache[cle] = cache;
  if (windowAvant === undefined) delete global.window; else global.window = windowAvant;
}

/* ————————————————————— verdict ————————————————————— */

if (process.exitCode) {
  console.log("FAILED test_secpol_js");
  process.exit(1);
}
console.log("OK " + checks + " checks");
