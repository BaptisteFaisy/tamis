"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — jumeau navigateur (browser twin) du cœur C. Mêmes moteurs, mêmes verdicts, */
/* mêmes rendus (CONTRACT.md §5) — vérifié par différentiel JS↔C (tests/test_js.sh). */
/* DOM-free : utilisable dans le navigateur (window.Tamis) et sous Node (module.exports). */

const VERDICT_NAMES = [
  "TAMIS_OK", "TAMIS_PARSE_ERROR", "TAMIS_UNTYPED_PROCESS", "TAMIS_MISSING_DEFAULT_SPAWN",
  "TAMIS_DEAD_RULE", "TAMIS_REDUNDANT_RULE", "TAMIS_OVERBROAD_ABILITY", "TAMIS_RANGE_INVALID",
  "TAMIS_PATH_FOUND", "TAMIS_INTERFERENCE_POSSIBLE", "TAMIS_PRIVILEGE_WIDENED",
  "TAMIS_PRIVILEGE_NARROWED", "TAMIS_INCONSISTENT",
];
const VERDICT_COUNT = VERDICT_NAMES.length;
const SEV = { INFO: 0, WARNING: 1, ERROR: 2 };
const SEV_NAMES = { 0: "info", 1: "warning", 2: "error" };
const DEFAULT_SEV = [0, 2, 1, 1, 0, 0, 1, 2, 2, 2, 2, 0, 2];
const SEV_LABEL = { 0: "INFO", 1: "WARN", 2: "ERROR" };
const CODE = {};
VERDICT_NAMES.forEach((n, i) => { CODE[n] = i; });

function verdictName(c) { return (c >= 0 && c < VERDICT_COUNT) ? VERDICT_NAMES[c] : "TAMIS_UNKNOWN"; }
function sevName(s) { return SEV_NAMES[s] || "unknown"; }

/* ————— Parseur du format provisoire (miroir de core/src/model.c) ————— */
function ParseError(line, message) { return { parseError: true, line, message }; }

function parseModel(text) {
  const lines = text.split("\n");
  const types = [], rules = [], untyped = [];
  let hasDefaultSpawn = false;
  const kwOf = (raw) => {
    const t = raw.replace(/^\s+/, "");
    if (t === "" || t[0] === "#") return null;
    return t.split(/\s+/)[0];
  };
  // Passe 1 : types + validation des mots-clés (comme count_pass/fill_types en C).
  lines.forEach((raw, i) => {
    const kw = kwOf(raw);
    if (kw === null) return;
    if (kw === "type") {
      const toks = raw.trim().split(/\s+/);
      if (!toks[1]) throw ParseError(i + 1, "type : nom manquant");
      types.push(toks[1]);
    } else if (["allow", "untyped", "default_spawn"].indexOf(kw) < 0) {
      throw ParseError(i + 1, "directive inconnue");
    }
  });
  const idx = {};
  types.forEach((t, i) => { if (!(t in idx)) idx[t] = i; });
  // Passe 2 : règles + untyped.
  lines.forEach((raw, i) => {
    const kw = kwOf(raw);
    if (kw === null) return;
    const toks = raw.trim().split(/\s+/);
    if (kw === "allow") {
      if (!toks[1]) throw ParseError(i + 1, "allow : source manquante");
      if (!toks[2]) throw ParseError(i + 1, "allow : cible manquante");
      const s = idx[toks[1]], d = idx[toks[2]];
      if (s === undefined || d === undefined) throw ParseError(i + 1, "allow : type non déclaré");
      const r = { src: s, dst: d, ability: null, id: null, line: i + 1 };
      for (let k = 3; k < toks.length; k++) {
        if (toks[k][0] === "@") r.id = toks[k].slice(1);
        else r.ability = toks[k];
      }
      rules.push(r);
    } else if (kw === "untyped") {
      if (!toks[1]) throw ParseError(i + 1, "untyped : nom manquant");
      untyped.push({ name: toks[1], line: i + 1 });
    } else if (kw === "default_spawn") {
      hasDefaultSpawn = true;
    }
  });
  return { types, idx, rules, untyped, hasDefaultSpawn };
}

function ruleLabel(r) { return r.id ? r.id : (r.ability ? r.ability : "?"); }
function typeIndex(m, name) { return (name in m.idx) ? m.idx[name] : -1; }

/* BFS s->d (miroir de tamis_reach_path). Retourne {found, path, rules, visits} ou {err}. */
function reachPath(m, s, d) {
  const nt = m.types.length;
  if (s < 0 || d < 0 || s >= nt || d >= nt) return { err: true };
  const visited = new Array(nt).fill(false);
  const parent = new Array(nt).fill(-1);
  const prule = new Array(nt).fill(-1);
  const queue = [s];
  let qh = 0, visits = 1;
  visited[s] = true;
  let found = (s === d);
  while (qh < queue.length && !found) {
    const u = queue[qh++];
    for (let r = 0; r < m.rules.length; r++) {
      if (m.rules[r].src !== u) continue;
      const v = m.rules[r].dst;
      if (visited[v]) continue;
      visited[v] = true; parent[v] = u; prule[v] = r; visits++;
      if (v === d) { found = true; break; }
      queue.push(v);
    }
  }
  if (!found) return { found: false, visits, path: null, rules: null };
  const nodes = [], ridx = [];
  let cur = d;
  nodes.unshift(cur);
  while (cur !== s) { ridx.unshift(prule[cur]); cur = parent[cur]; nodes.unshift(cur); }
  return { found: true, visits, path: nodes.map((i) => m.types[i]), rules: ridx.map((ri) => ruleLabel(m.rules[ri])) };
}

function V(o) {
  return Object.assign({ code: 0, sev: 0, file: null, line: 0, column: 0, message: null,
    src: null, dst: null, path: null, rules: null }, o);
}

/* ————— Moteurs (miroirs des core/src/*.c) ————— */
function reach(m, src, dst, file) {
  const s = typeIndex(m, src), d = typeIndex(m, dst);
  if (s < 0 || d < 0) return { err: true };
  const rp = reachPath(m, s, d);
  if (!rp.found) return { verdicts: [] };
  return { verdicts: [V({ code: CODE.TAMIS_PATH_FOUND, sev: SEV.ERROR, file, src, dst,
    path: rp.path, rules: rp.rules, message: "chemin de privilège atteignable" })] };
}

function proof(m, src, dst, file) {
  const s = typeIndex(m, src), d = typeIndex(m, dst);
  if (s < 0 || d < 0) return { err: true };
  const rp = reachPath(m, s, d);
  if (rp.found) return { verdicts: [V({ code: CODE.TAMIS_INTERFERENCE_POSSIBLE, sev: SEV.ERROR,
    file, src, dst, path: rp.path, rules: rp.rules, message: "interférence possible : chemin de privilège trouvé" })] };
  return { verdicts: [V({ code: CODE.TAMIS_OK, sev: SEV.INFO, file, src, dst,
    message: `non-interférence prouvée : ${src} n'atteint pas ${dst} — ${rp.visits} nœud(s) visité(s), parcours exhaustif` })] };
}

function lint(m, file) {
  const vs = [];
  m.untyped.forEach((u) => vs.push(V({ code: CODE.TAMIS_UNTYPED_PROCESS, sev: SEV.WARNING,
    file, line: u.line, message: `processus non typé : ${u.name}` })));
  if (!m.hasDefaultSpawn) vs.push(V({ code: CODE.TAMIS_MISSING_DEFAULT_SPAWN, sev: SEV.WARNING,
    file, message: "default_spawn_type absent : les processus spawned ne changent pas de type (trou S7)" }));
  return { verdicts: vs };
}

function allPairs(m) {
  const nt = m.types.length, mat = [];
  for (let i = 0; i < nt; i++) { mat.push([]);
    for (let j = 0; j < nt; j++) mat[i].push(reachPath(m, i, j).found ? 1 : 0); }
  return mat;
}
function matEq(a, b) { return JSON.stringify(a) === JSON.stringify(b); }

function minimize(m, file) {
  const vs = [];
  const kept = [];
  let redundant = 0;
  const minm = { types: m.types, idx: m.idx, rules: kept, untyped: [], hasDefaultSpawn: m.hasDefaultSpawn };
  m.rules.forEach((r) => {
    const rp = reachPath(minm, r.src, r.dst);
    if (rp.found) {
      vs.push(V({ code: CODE.TAMIS_REDUNDANT_RULE, sev: SEV.INFO, file, line: r.line, rules: [ruleLabel(r)],
        message: `règle redondante ${ruleLabel(r)} : ${m.types[r.src]} atteint déjà ${m.types[r.dst]} sans elle` }));
      redundant++;
    } else kept.push(r);
  });
  const nt = m.types.length;
  const same = (nt === 0) || matEq(allPairs(m), allPairs(minm));
  vs.push(V({ code: same ? CODE.TAMIS_OK : CODE.TAMIS_INCONSISTENT, sev: same ? SEV.INFO : SEV.ERROR, file,
    message: `minimisation ${same ? "équivalente" : "NON équivalente (ANOMALIE)"} : ${redundant} règle(s) retirée(s), ${nt * nt} paire(s) d'accessibilité ${same ? "identiques" : "DIFFÉRENTES"}` }));
  return { verdicts: vs };
}

function abilityOf(r) { return r.ability ? r.ability : ""; }
function hasEdge(m, src, dst, ability) {
  return m.rules.some((r) => m.types[r.src] === src && m.types[r.dst] === dst && abilityOf(r) === ability);
}
function diff(base, cand, file) {
  const vs = [];
  cand.rules.forEach((r) => {
    const s = cand.types[r.src], d = cand.types[r.dst];
    if (!hasEdge(base, s, d, abilityOf(r))) vs.push(V({ code: CODE.TAMIS_PRIVILEGE_WIDENED, sev: SEV.ERROR,
      file, src: s, dst: d, message: `élargissement de privilège : ${s} -> ${d} [${abilityOf(r)}]` }));
  });
  base.rules.forEach((r) => {
    const s = base.types[r.src], d = base.types[r.dst];
    if (!hasEdge(cand, s, d, abilityOf(r))) vs.push(V({ code: CODE.TAMIS_PRIVILEGE_NARROWED, sev: SEV.INFO,
      file, src: s, dst: d, message: `réduction de privilège : ${s} -> ${d} [${abilityOf(r)}]` }));
  });
  return { verdicts: vs };
}

function explain(m, src, dst, file) {
  const s = typeIndex(m, src), d = typeIndex(m, dst);
  if (s < 0 || d < 0) return { err: true };
  const rp = reachPath(m, s, d);
  if (rp.found) {
    let chain = rp.path[0];
    for (let i = 0; i < rp.rules.length; i++) chain += ` →(${rp.rules[i]}) ${rp.path[i + 1]}`;
    const last = rp.rules.length ? rp.rules[rp.rules.length - 1] : "la dernière règle";
    const vs = reach(m, src, dst, file).verdicts;
    vs.push(V({ code: CODE.TAMIS_OK, sev: SEV.INFO, file, src, dst,
      message: `Pourquoi ce refus : ${src} atteint ${dst} via la chaîne ${chain}. Correctif minimal : retirer la règle ${last} (dernière arête avant ${dst}) tue ce chemin ; relancer \`tamis proof --src ${src} --dst ${dst}\` pour obtenir le certificat de non-interférence.` }));
    return { verdicts: vs };
  }
  return { verdicts: [V({ code: CODE.TAMIS_OK, sev: SEV.INFO, file, src, dst,
    message: `Aucun chemin ${src} → ${dst} : non-interférence prouvée (${rp.visits} nœud(s) visité(s), parcours exhaustif). Périmètre de complétude : flux explicites de type à type du modèle. Angles morts bornés (TAM-21) : flux IPC indirects (MsgSend/ConnectAttach), permissions de fichiers et transitions root→non-root — NON couverts, à traiter hors Tamis.` })] };
}

const TamisCore = {
  VERDICT_NAMES, VERDICT_COUNT, SEV, SEV_NAMES, DEFAULT_SEV, SEV_LABEL, CODE,
  verdictName, sevName, parseModel, typeIndex, reachPath, ruleLabel,
  reach, proof, lint, minimize, diff, explain, allPairs,
};
if (typeof module !== "undefined" && module.exports) module.exports = TamisCore;
if (typeof window !== "undefined") window.TamisCore = TamisCore;
