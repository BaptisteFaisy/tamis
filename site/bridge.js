"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — onglet « Banc réel » : pilote le pont local (vrai cœur C + vraie VM QNX). */
/* Autonome : lit l'état du Playground dans le DOM, aucun couplage avec app.js. */
(function () {
  const C = window.TamisCore;
  const R = window.TamisRender;
  const Run = window.TamisRun;
  const $ = (id) => document.getElementById(id);
  if (!$("bridgeConnect") || !C || !R || !Run) return;

  const bridge = { url: "http://127.0.0.1:8787", token: "", online: false };
  const LABEL = "policy.mpf";

  function lang() { return document.body.classList.contains("lang-fr") ? "fr" : "en"; }
  function t(fr, en) { return lang() === "fr" ? fr : en; }

  function badge(el, texte, cls) {
    el.textContent = texte;
    el.className = "badge " + cls;
    el.hidden = false;
  }

  async function call(path, corps, ms) {
    const opt = { method: corps ? "POST" : "GET", headers: { "X-Tamis-Token": bridge.token } };
    if (corps) {
      opt.headers["Content-Type"] = "application/json";
      opt.body = JSON.stringify(corps);
    }
    const ctl = new AbortController();
    const timer = setTimeout(() => ctl.abort(), ms || 20000);
    opt.signal = ctl.signal;
    try {
      const rep = await fetch(bridge.url.replace(/\/+$/, "") + path, opt);
      return await rep.json();
    } catch (e) {
      return { error: t("pont injoignable — ", "bridge unreachable — ") + e.message };
    } finally {
      clearTimeout(timer);
    }
  }

  /* ——— état courant du Playground, lu dans le DOM ——— */
  function actif(sel) {
    const b = document.querySelector(sel + " button.active");
    return b ? (b.dataset.cmd || b.dataset.fmt) : null;
  }

  function requete() {
    return {
      cmd: actif("#toolbar") || "lint",
      policy: $("policyA").value,
      policy2: $("policyB").value,
      src: $("src").value || null,
      dst: $("dst").value || null,
      label: LABEL,
    };
  }

  /* ——— rendu du jumeau JS, pour comparaison (parse/exécution partagés) ——— */
  function jsonJS(rq) {
    const r = Run.executer({ cmd: rq.cmd, policy: rq.policy, policy2: rq.policy2,
      src: rq.src, dst: rq.dst, label: rq.label });
    if (r.nonComparable) return "(commande non comparable)";
    if (r.phase === "policy2") return "(politique candidate invalide)";
    if (r.errType) return "(type inconnu)";
    return Run.rendre("json", r.verdicts, null);
  }

  /* ——— connexion ——— */
  async function connect() {
    bridge.url = $("bridgeUrl").value.trim() || bridge.url;
    bridge.token = $("bridgeToken").value.trim();
    badge($("bridgeState"), t("connexion…", "connecting…"), "warn");
    $("bridgeInfo").textContent = "";
    const ping = await call("/ping", null, 5000);
    if (!ping || ping.ok !== true) {
      bridge.online = false;
      badge($("bridgeState"), t("injoignable", "unreachable"), "ko");
      $("bridgeInfo").textContent = t(
        "Aucun pont sur " + bridge.url + " — lance `python3 tools/tamis-bridge.py` dans le dépôt.",
        "No bridge on " + bridge.url + " — run `python3 tools/tamis-bridge.py` in the repository.");
      return;
    }
    const pre = await call("/presets", null, 8000);
    if (pre && pre.error) {
      bridge.online = false;
      badge($("bridgeState"), t("jeton refusé", "token rejected"), "ko");
      $("bridgeInfo").textContent = pre.error;
      return;
    }
    bridge.online = true;
    badge($("bridgeState"), t("connecté", "connected"), "ok");
    $("bridgeInfo").textContent = t("Pont v", "Bridge v") + (ping.version || "?") +
      " · " + (ping.cli_c ? t("cœur C construit", "C core built") : t("cœur C à construire", "C core to build")) +
      " · QNX : " + (ping.qnx && ping.qnx.configured
        ? (ping.qnx.running ? t("configuré, en marche", "configured, running") : t("configuré, arrêté", "configured, stopped"))
        : t("non configuré", "not configured"));
    $("benchPanels").hidden = false;
    presets((pre && pre.presets) || []);
    qnxStatus();
  }

  /* ——— comparaison C réel ↔ jumeau JS ——— */
  async function compare() {
    if (!bridge.online) return;
    const rq = requete();
    $("runCmd").textContent = "tamis " + rq.cmd + (rq.src ? " --src " + rq.src + " --dst " + rq.dst : "");
    if (rq.cmd === "view") {
      $("outC").textContent = t("view n'est pas comparable (sortie graphe).", "view is not comparable (graph output).");
      $("outJS").textContent = "";
      return;
    }
    $("outC").textContent = t("… exécution du vrai binaire C …", "… running the real C binary …");
    const res = await call("/tamis", Object.assign({}, rq, { format: "json" }), 60000);
    if (res.error) {
      $("outC").textContent = "ERREUR : " + res.error;
      badge($("cmpVerdict"), t("échec", "failed"), "ko");
      return;
    }
    const cTxt = (res.stdout || "").trim();
    const jsTxt = jsonJS(rq).trim();
    $("outC").textContent = "# " + (res.argv || "") + "\n# exit " + res.exit + "\n\n" + cTxt;
    $("outJS").textContent = jsTxt;
    const same = cTxt === jsTxt;
    badge($("cmpVerdict"),
      same ? t("identiques ✓ (différentiel live)", "identical ✓ (live differential)")
           : t("DIFFÉRENTS ✗", "DIFFERENT ✗"),
      same ? "ok" : "ko");
  }

  /* ——— banc QNX ——— */
  function presets(liste) {
    const box = $("qnxPresets");
    box.innerHTML = "";
    liste.forEach((p) => {
      const b = document.createElement("button");
      b.textContent = p.id;
      b.title = p.label;
      b.onclick = () => qnxExec(p.id, p.label);
      box.appendChild(b);
    });
  }

  async function qnxStatus() {
    const s = await call("/qnx/status", null, 15000);
    if (s.error) { badge($("qnxState"), s.error, "ko"); return; }
    if (!s.configured) { badge($("qnxState"), t("VM non configurée", "VM not configured"), "warn"); return; }
    badge($("qnxState"), s.running
      ? (s.ssh_ready ? t("QNX en marche, SSH prêt", "QNX running, SSH ready") : t("QNX en marche, SSH pas prêt", "QNX up, SSH not ready"))
      : t("QNX arrêté", "QNX stopped"), s.ssh_ready ? "ok" : (s.running ? "warn" : "warn"));
  }

  async function qnxExec(id, label) {
    $("qnxOut").textContent = t("… exécution réelle dans QNX : ", "… running for real in QNX: ") + (label || id) + " …";
    const r = await call("/qnx/exec", { preset: id }, 90000);
    if (r.error) { $("qnxOut").textContent = "ERREUR : " + r.error; return; }
    $("qnxOut").textContent = "$ " + r.remote_cmd + "\n\n" + (r.stdout || "") +
      (r.stderr ? "\n[stderr] " + r.stderr : "") + "\n(exit " + r.exit + ")";
  }

  $("bridgeConnect").onclick = connect;
  $("runReal").onclick = compare;
  $("qnxRefresh").onclick = qnxStatus;
  $("qnxBoot").onclick = async () => {
    $("qnxOut").textContent = t("… démarrage de la VM QNX (jusqu'à 3 min) …", "… booting the QNX VM (up to 3 min) …");
    badge($("qnxState"), t("démarrage…", "booting…"), "warn");
    const r = await call("/qnx/boot", {}, 240000);
    $("qnxOut").textContent = JSON.stringify(r, null, 2);
    qnxStatus();
  };
  $("qnxStop").onclick = async () => {
    const r = await call("/qnx/stop", {}, 60000);
    $("qnxOut").textContent = (r.stdout || "") + (r.stderr || "") + (r.error || "");
    qnxStatus();
  };

  /* détection silencieuse au chargement : si un pont tourne déjà, on le signale */
  (async () => {
    const ping = await call("/ping", null, 2500);
    if (ping && ping.ok === true) {
      badge($("bridgeState"), t("pont détecté — jeton requis", "bridge detected — token required"), "warn");
    }
  })();
})();
