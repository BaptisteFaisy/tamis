"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — rendus texte / JSON / Markdown d'audit (CONTRACT.md §5). */
/* Échappement et ordre des clés identiques au cœur C : le JSON est comparable octet pour octet. */

function jsonString(s) {
  let out = '"';
  for (const ch of String(s)) {
    const cp = ch.codePointAt(0);
    if (ch === '"') out += '\\"';
    else if (ch === "\\") out += "\\\\";
    else if (cp === 0x08) out += "\\b";
    else if (cp === 0x0C) out += "\\f";
    else if (cp === 0x0A) out += "\\n";
    else if (cp === 0x0D) out += "\\r";
    else if (cp === 0x09) out += "\\t";
    else if (cp < 0x20) out += "\\u" + cp.toString(16).padStart(4, "0");
    else out += ch;
  }
  return out + '"';
}

function jsonArray(arr) {
  return "[" + arr.map((x) => jsonString(x === null || x === undefined ? "" : x)).join(",") + "]";
}

function jsonObj(v, verdictName, sevName) {
  const p = [];
  p.push('"code":' + jsonString(verdictName(v.code)));
  p.push('"sev":' + jsonString(sevName(v.sev)));
  if (v.src !== null && v.src !== undefined) p.push('"src":' + jsonString(v.src));
  if (v.dst !== null && v.dst !== undefined) p.push('"dst":' + jsonString(v.dst));
  if (v.path && v.path.length) {
    p.push('"hops":' + (v.path.length - 1));
    p.push('"path":' + jsonArray(v.path));
  }
  if (v.rules && v.rules.length) p.push('"rules":' + jsonArray(v.rules));
  if (v.file !== null && v.file !== undefined) p.push('"file":' + jsonString(v.file));
  if (v.line > 0) p.push('"line":' + v.line);
  if (v.column > 0) p.push('"column":' + v.column);
  p.push('"message":' + jsonString(v.message === null || v.message === undefined ? "" : v.message));
  return "{" + p.join(",") + "}";
}

function position(v) {
  let pos = (v.file !== null && v.file !== undefined) ? v.file : "?";
  if (v.line > 0) pos += ":" + v.line;
  if (v.column > 0) pos += ":" + v.column;
  return pos;
}

function detail(v) {
  if (v.path && v.path.length) {
    const src = (v.src !== null && v.src !== undefined) ? v.src : "?";
    const dst = (v.dst !== null && v.dst !== undefined) ? v.dst : "?";
    let d = src + " -> " + dst + " (" + (v.path.length - 1) + " sauts)";
    if (v.rules && v.rules.length) d += " : " + v.rules.join(",");
    return d;
  }
  return (v.message !== null && v.message !== undefined) ? v.message : "";
}

function renderText(vs, verdictName, sevName, sevLabel) {
  let out = "";
  vs.forEach((v) => {
    const prefix = (sevLabel[v.sev] || "INFO").padEnd(5) + "  ";
    out += prefix + verdictName(v.code) + "  " + position(v) + "  " + detail(v) + "\n";
  });
  const errs = vs.filter((v) => v.sev === 2).length;
  out += errs === 0 ? "OK\n" : "FAILED " + errs + " error(s)\n";
  return out;
}

function renderJson(vs, verdictName, sevName) {
  return vs.map((v) => jsonObj(v, verdictName, sevName) + "\n").join("");
}

function mdMeta(meta) {
  const m = meta || {};
  return "# Rapport d'audit Tamis\n\n" +
    "- Politique : " + (m.policy_file || "inconnu") + "\n" +
    "- Empreinte SHA-256 : " + (m.policy_sha256 || "inconnu") + "\n" +
    "- Horodatage (UTC) : " + (m.timestamp_utc || "inconnu") + "\n" +
    "- Version de Tamis : " + (m.tamis_version || "inconnu") + "\n\n";
}

function mdBullets(v) {
  let s = "- Position : " + position(v) + "\n";
  s += "- Message : " + ((v.message !== null && v.message !== undefined) ? v.message : "") + "\n";
  if (v.path && v.path.length) {
    const src = (v.src !== null && v.src !== undefined) ? v.src : "?";
    const dst = (v.dst !== null && v.dst !== undefined) ? v.dst : "?";
    s += "- Chemin : " + src + " → " + dst + " (" + (v.path.length - 1) + " sauts) : " + v.path.join(" → ") + "\n";
  }
  if (v.rules && v.rules.length) s += "- Règles : " + v.rules.join(", ") + "\n";
  return s;
}

function renderMarkdown(vs, meta, verdictName, sevName, defaultSev, verdictCount) {
  let s = mdMeta(meta);
  const err = vs.filter((v) => v.sev === 2).length;
  const warn = vs.filter((v) => v.sev === 1).length;
  const info = vs.filter((v) => v.sev === 0).length;
  s += "## Synthèse\n\n| Sévérité | Nombre |\n|---|---|\n";
  s += "| error | " + err + " |\n| warning | " + warn + " |\n| info | " + info + " |\n\n";
  s += "## Verdicts\n\n";
  for (let c = 0; c < verdictCount; c++) {
    const vs2 = vs.filter((v) => v.code === c);
    if (!vs2.length) continue;
    s += "### " + verdictName(c) + " — " + sevName(defaultSev[c]) + " (×" + vs2.length + ")\n";
    vs2.forEach((v) => { s += mdBullets(v); });
    s += "\n";
  }
  s += "## Propriétés vérifiées\n\n";
  s += "(une section par propriété : énoncé, méthode, corpus, résultat, contre-exemple)\n";
  return s;
}

function exitCode(vs) { return vs.some((v) => v.sev === 2) ? 1 : 0; }

const TamisRender = {
  jsonString, jsonArray, jsonObj, position, detail,
  renderText, renderJson, renderMarkdown, exitCode,
};
if (typeof module !== "undefined" && module.exports) module.exports = TamisRender;
if (typeof window !== "undefined") window.TamisRender = TamisRender;
