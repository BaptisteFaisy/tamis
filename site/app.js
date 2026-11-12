"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — playground : câblage UI + graphe de privilèges interactif. */
/* Utilise le jumeau JS (tamis-core.js / tamis-render.js), identique au cœur C. */
(function () {
  const C = window.TamisCore;
  const R = window.TamisRender;
  const $ = (id) => document.getElementById(id);
  const SVGNS = "http://www.w3.org/2000/svg";

  /* FIXTURES:START */
const EXAMPLES = {"attaque": "# Politique de démonstration — format PROVISOIRE (model text format, voir core/src/model.c).\n# À remplacer par du vrai secpol dès que la grammaire est gelée (lot L1, docs/GRAMMAR.md).\n#\n# Scénario : un tiers applicatif peut atteindre la pompe critique en 3 sauts\n# (app_tiers -> journal -> diag -> pompe_critique). C'est le chemin d'attaque de la démo.\ntype app_tiers\ntype journal\ntype diag\ntype pompe_critique\ntype reseau\n\nallow app_tiers journal msg @R112\nallow journal diag msg @R87\nallow diag pompe_critique control @R9\nallow reseau journal msg @R10\n\n# Doublon : même arête app_tiers -> journal déjà accordée par R112 (redondante, vu par minimize).\nallow app_tiers journal msg @R999\n\n# Processus sans type (lint TAM-06) :\nuntyped /bin/legacy\nuntyped /bin/foo\n\n# Pas de default_spawn_type déclaré -> le lint le signale (trou S7).\n", "durcie": "# Politique DURCIE — format PROVISOIRE. Version corrigée de attaque.mpf.\n# Le chemin d'attaque vers pompe_critique est coupé (R9 supprimée), le doublon R999 retiré,\n# et default_spawn_type déclaré. Sert de cible au diff sémantique (TAM-08).\ntype app_tiers\ntype journal\ntype diag\ntype pompe_critique\ntype reseau\n\nallow app_tiers journal msg @R112\nallow journal diag msg @R87\nallow reseau journal msg @R10\n\n# R9 (diag -> pompe_critique) SUPPRIMÉE : plus aucun chemin app_tiers -> pompe_critique.\n# R999 (doublon de R112) SUPPRIMÉE.\n\ndefault_spawn\n"};
/* FIXTURES:END */

  const state = { cmd: "lint", fmt: "text", verdicts: [], model: null, modelB: null, fileA: "policy.mpf" };

  /* ——— navigation par onglets ——— */
  document.querySelectorAll("#nav button").forEach((b) => {
    b.onclick = () => {
      document.querySelectorAll("#nav button").forEach((x) => x.classList.toggle("active", x === b));
      document.querySelectorAll(".tab").forEach((s) => s.classList.toggle("active", s.id === b.dataset.tab));
      if (b.dataset.tab === "playground") drawGraph();
    };
  });

  document.querySelectorAll("#fmt button").forEach((b) => {
    b.onclick = () => {
      state.fmt = b.dataset.fmt;
      document.querySelectorAll("#fmt button").forEach((x) => x.classList.toggle("active", x === b));
      show();
    };
  });

  document.querySelectorAll("#toolbar button").forEach((b) => { b.onclick = () => run(b.dataset.cmd); });

  $("loadExample").onclick = () => {
    const name = $("example").value;
    $("policyA").value = EXAMPLES[name] || "";
    if (name === "attaque") $("policyB").value = EXAMPLES.durcie || "";
    refreshTypes();
    run(state.cmd);
  };

  $("copy").onclick = () => {
    const t = $("output").textContent;
    if (navigator.clipboard) navigator.clipboard.writeText(t);
    $("copy").textContent = "Copié ✓";
    setTimeout(() => { $("copy").textContent = "Copier"; }, 1200);
  };

  ["policyA", "policyB", "src", "dst"].forEach((id) => {
    $(id).addEventListener("change", () => { refreshTypes(); run(state.cmd); });
  });
  $("policyA").addEventListener("input", debounce(() => { refreshTypes(); }, 400));

  function debounce(fn, ms) {
    let t;
    return function () { clearTimeout(t); t = setTimeout(fn, ms); };
  }

  /* ——— analyse ——— */
  function parseErrVerdict(e, file) {
    return [{ code: 1, sev: 2, file, line: e.line || 0, column: 0,
      message: e.message || "erreur de syntaxe", src: null, dst: null, path: null, rules: null }];
  }

  function refreshTypes() {
    let m;
    try { m = C.parseModel($("policyA").value); }
    catch (e) { state.model = null; drawGraph(); return; }
    state.model = m;
    const names = m.types;
    [["src", 0], ["dst", names.length - 1]].forEach(([id, fallback]) => {
      const sel = $(id);
      const prev = sel.value;
      sel.innerHTML = "";
      names.forEach((n) => {
        const o = document.createElement("option");
        o.value = n; o.textContent = n;
        sel.appendChild(o);
      });
      sel.value = (prev && names.indexOf(prev) >= 0) ? prev : (names[fallback] || "");
    });
    drawGraph();
  }

  function run(cmd) {
    state.cmd = cmd;
    document.querySelectorAll("#toolbar button").forEach((b) =>
      b.classList.toggle("active", b.dataset.cmd === cmd));
    $("panelB").hidden = (cmd !== "diff");

    let mA;
    try { mA = C.parseModel($("policyA").value); }
    catch (e) { state.verdicts = parseErrVerdict(e, state.fileA); return finish(); }
    state.model = mA;
    const src = $("src").value;
    const dst = $("dst").value;
    let res;
    try {
      if (cmd === "lint") res = C.lint(mA, state.fileA);
      else if (cmd === "reach") res = C.reach(mA, src, dst, state.fileA);
      else if (cmd === "proof") res = C.proof(mA, src, dst, state.fileA);
      else if (cmd === "explain") res = C.explain(mA, src, dst, state.fileA);
      else if (cmd === "minimize") res = C.minimize(mA, state.fileA);
      else if (cmd === "diff") {
        let mB;
        try { mB = C.parseModel($("policyB").value); }
        catch (e) { state.verdicts = parseErrVerdict(e, "candidate.mpf"); return finish(); }
        state.modelB = mB;
        res = C.diff(mA, mB, state.fileA);
      } else res = { verdicts: [] };
    } catch (e) { state.verdicts = parseErrVerdict(e, state.fileA); return finish(); }

    if (res.err) {
      state.verdicts = [{ code: 12, sev: 2, file: state.fileA, line: 0, column: 0,
        message: "type inconnu : src ou dst absent du modèle", src: src, dst: dst, path: null, rules: null }];
    } else {
      state.verdicts = res.verdicts;
    }
    finish();
  }

  function finish() {
    show();
    drawGraph();
  }

  function show() {
    const vs = state.verdicts || [];
    const out = $("output");
    if (state.fmt === "json") {
      out.textContent = R.renderJson(vs, C.verdictName, C.sevName) || "(aucun verdict)";
    } else if (state.fmt === "md") {
      out.textContent = R.renderMarkdown(vs, meta(), C.verdictName, C.sevName, C.DEFAULT_SEV, C.VERDICT_COUNT);
    } else {
      out.textContent = vs.length
        ? R.renderText(vs, C.verdictName, C.sevName, C.SEV_LABEL)
        : "OK\n";
    }
    const badge = $("exitBadge");
    const errs = vs.filter((v) => v.sev === 2).length;
    const warns = vs.filter((v) => v.sev === 1).length;
    badge.hidden = false;
    badge.className = "badge " + (errs ? "ko" : (warns ? "warn" : "ok"));
    badge.textContent = "exit " + (errs ? 1 : 0) + (errs ? " · " + errs + " error" : (warns ? " · " + warns + " warning" : " · sain"));
  }

  function meta() {
    return {
      policy_file: state.fileA,
      policy_sha256: "calculée côté CLI (tools/sha256.c) — le navigateur ne la calcule pas",
      timestamp_utc: new Date().toISOString().replace(/\.\d+Z$/, "Z"),
      tamis_version: "0.1.0 (jumeau navigateur)",
    };
  }

  /* ——— graphe de privilèges ——— */
  function svgEl(name, attrs, parent) {
    const e = document.createElementNS(SVGNS, name);
    for (const k in attrs) e.setAttribute(k, attrs[k]);
    if (parent) parent.appendChild(e);
    return e;
  }

  function drawGraph() {
    const host = $("graph");
    const status = $("graphStatus");
    host.innerHTML = "";
    const m = state.model;
    if (!m || !m.types.length) { status.textContent = ""; return; }

    const nodes = m.types;
    const edges = m.rules.map((r) => ({ src: nodes[r.src], dst: nodes[r.dst], rule: C.ruleLabel(r) }));
    const idx = {};
    nodes.forEach((n, i) => { idx[n] = i; });

    /* couches = plus long chemin depuis une racine (relaxation |V| fois) */
    const L = nodes.map(() => 0);
    for (let it = 0; it < nodes.length; it++) {
      edges.forEach((e) => {
        const u = idx[e.src], v = idx[e.dst];
        if (L[v] < L[u] + 1 && L[u] + 1 <= nodes.length) L[v] = L[u] + 1;
      });
    }
    const byLayer = {};
    nodes.forEach((n, i) => { (byLayer[L[i]] = byLayer[L[i]] || []).push(i); });
    const maxLayer = Math.max.apply(null, Object.keys(byLayer).map(Number).concat([0]));
    const xGap = 175, yGap = 76, mx = 70, my = 46;
    let maxRows = 1;
    Object.keys(byLayer).forEach((k) => { maxRows = Math.max(maxRows, byLayer[k].length); });
    const pos = nodes.map(() => ({ x: 0, y: 0 }));
    Object.keys(byLayer).forEach((k) => {
      byLayer[k].forEach((ni, r) => { pos[ni] = { x: mx + (+k) * xGap, y: my + r * yGap }; });
    });

    const svg = svgEl("svg", {
      width: mx * 2 + maxLayer * xGap + 60,
      height: my * 2 + (maxRows - 1) * yGap + 34,
    }, host);
    const defs = svgEl("defs", {}, svg);
    [["arrow", "#3a4152"], ["arrowHot", "#ff5d5d"]].forEach(([id, color]) => {
      const mk = svgEl("marker", { id, markerWidth: 10, markerHeight: 10, refX: 8, refY: 3, orient: "auto" }, defs);
      svgEl("path", { d: "M0,0 L8,3 L0,6 Z", fill: color }, mk);
    });

    /* focus : même BFS que le moteur (zéro duplication de logique) */
    const s = $("src").value, d = $("dst").value;
    let hotNodes = new Set(), hotEdges = new Set(), pathInfo = null;
    if (s && d && idx[s] !== undefined && idx[d] !== undefined) {
      const rp = C.reachPath(m, idx[s], idx[d]);
      if (rp.found) {
        const pidx = rp.path.map((n) => idx[n]);
        hotNodes = new Set(pidx);
        for (let i = 0; i < pidx.length - 1; i++) {
          edges.forEach((e, ei) => { if (idx[e.src] === pidx[i] && idx[e.dst] === pidx[i + 1]) hotEdges.add(ei); });
        }
        pathInfo = { n: pidx.length - 1, rules: rp.rules };
      } else pathInfo = { n: -1, visits: rp.visits };
    }

    edges.forEach((e, ei) => {
      const a = pos[idx[e.src]], b = pos[idx[e.dst]];
      if (!a || !b) return;
      const hot = hotEdges.has(ei);
      const midX = (a.x + b.x) / 2;
      svgEl("path", {
        class: "edge" + (hot ? " hot" : ""),
        d: "M" + a.x + "," + a.y + " C" + midX + "," + a.y + " " + midX + "," + b.y + " " + b.x + "," + b.y,
        "marker-end": hot ? "url(#arrowHot)" : "url(#arrow)",
      }, svg);
      const t = svgEl("text", { class: "elabel" + (hot ? " hot" : ""), x: midX, y: (a.y + b.y) / 2 - 5, "text-anchor": "middle" }, svg);
      t.textContent = e.rule;
    });

    nodes.forEach((n, i) => {
      const g = svgEl("g", { class: "node" + (hotNodes.has(i) ? " focus" : ""), transform: "translate(" + pos[i].x + "," + pos[i].y + ")" }, svg);
      svgEl("circle", { r: 21, cx: 0, cy: 0 }, g);
      const t = svgEl("text", { x: 0, y: 34, "text-anchor": "middle" }, g);
      t.textContent = n;
    });

    status.textContent = pathInfo
      ? (pathInfo.n >= 0
        ? "⚠ chemin " + s + " → " + d + " en " + pathInfo.n + " saut(s) : " + pathInfo.rules.join(", ")
        : "✓ aucun chemin " + s + " → " + d + " (" + pathInfo.visits + " nœuds visités)")
      : nodes.length + " types, " + edges.length + " règles";
  }

  /* ——— démarrage ——— */
  $("policyA").value = EXAMPLES.attaque || "";
  $("policyB").value = EXAMPLES.durcie || "";
  refreshTypes();
  run("lint");
})();
