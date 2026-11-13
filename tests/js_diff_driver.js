"use strict";
/* SPDX-License-Identifier: MIT */
/* Pilote Node du différentiel JS↔C : rend les verdicts du jumeau navigateur en JSON, */
/* dans le même format que `build/tamis <cmd> … --format json`. */
/* Usage : node tests/js_diff_driver.js <cmd> <fichier> [fichier2] [src] [dst] */
const fs = require("fs");
const path = require("path");
const C = require(path.join(__dirname, "..", "site", "tamis-core.js"));
const R = require(path.join(__dirname, "..", "site", "tamis-render.js"));
const S = require(path.join(__dirname, "..", "site", "tamis-secpol.js"));

const [cmd, file, file2, src, dst] = process.argv.slice(2);

function emit(verdicts) {
  process.stdout.write(R.renderJson(verdicts, C.verdictName, C.sevName));
}

/* Le CLI C choisit le parseur sur l'extension .secpol : le driver fait de même. */
function estSecpol(f) { return /\.secpol$/.test(f); }

function parseOrErr(f) {
  const texte = fs.readFileSync(f, "utf8");
  if (estSecpol(f)) {
    const r = S.parseSecpol(texte);
    if (r.error) {
      return { err: [{ code: 1, sev: 2, file: f, line: r.error.line || 0,
        column: r.error.column || 0, message: r.error.message || "erreur de syntaxe",
        src: null, dst: null, path: null, rules: null }] };
    }
    return { m: r.model };
  }
  try {
    return { m: C.parseModel(texte) };
  } catch (e) {
    return { err: [{ code: 1, sev: 2, file: f, line: e.line || 0, column: 0,
      message: e.message || "erreur de syntaxe", src: null, dst: null, path: null, rules: null }] };
  }
}

const A = parseOrErr(file);
if (A.err) { emit(A.err); process.exit(0); }

let res;
if (cmd === "lint") res = C.lint(A.m, file);
else if (cmd === "minimize") res = C.minimize(A.m, file);
else if (cmd === "reach") res = C.reach(A.m, src, dst, file);
else if (cmd === "proof") res = C.proof(A.m, src, dst, file);
else if (cmd === "explain") res = C.explain(A.m, src, dst, file);
else if (cmd === "diff") {
  const B = parseOrErr(file2);
  if (B.err) { emit(B.err); process.exit(0); }
  res = C.diff(A.m, B.m, file);
} else {
  process.stderr.write("KO commande inconnue : " + cmd + "\n");
  process.exit(2);
}

if (res.err) {
  /* Le CLI C n'émet rien sur stdout dans ce cas (message sur stderr, code 1). */
  process.stderr.write("KO type inconnu\n");
  process.exit(1);
}
emit(res.verdicts);