"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — onglet « Invité QNX en direct » : rejouer une preuve DANS la vraie cible QNX 8.0.
   Autonomous: no dependency on app.js. Talks to the Tamis emulator service (banc/service.py),
   exposed over HTTPS by tailscale funnel. The service executes a CLOSED list of steps in the
   guest (QNX 8.0 under QEMU) and returns its output — compared byte for byte with the host. */
(function () {
  const $ = (id) => document.getElementById(id);
  const live = document.getElementById("live");
  if (!live) return;

  // Port 8443 : Tailscale Funnel ne publie que 443, 8443 et 10000 — un montage sur
  // :8094 n'est joignable que depuis le tailnet (voir docs/VITRINE-PUBLIQUE.md).
  const SERVICE = window.TAMIS_EMUL_URL || "https://r630-debian.tail3a8bdf.ts.net:8443/tamis-api";
  const DELAI = 150000;          // l'invité peut avoir besoin de démarrer

  function lang() { return document.body.classList.contains("lang-fr") ? "fr" : "en"; }
  function t(fr, en) { return lang() === "fr" ? fr : en; }

  function el(tag, cls, html) {
    const e = document.createElement(tag);
    if (cls) e.className = cls;
    if (html !== undefined) e.innerHTML = html;
    return e;
  }

  function champ(label, html) {
    return el("div", "row", `<span class="hint">${label}</span>` + html);
  }

  function versUrl(suite) { return SERVICE.replace(/\/+$/, "") + suite; }

  async function appeler(suite, ms) {
    const ctl = new AbortController();
    const minuteur = setTimeout(() => ctl.abort(), ms || 30000);
    try {
      const rep = await fetch(versUrl(suite), { signal: ctl.signal, cache: "no-store" });
      const corps = await rep.json();
      return { ok: rep.ok, corps: corps };
    } catch (e) {
      return { ok: false, corps: { erreur: e.message } };
    } finally {
      clearTimeout(minuteur);
    }
  }

  /* ——— panneau injecté dans l'onglet « Invité QNX en direct » ——— */
  const panneau = el("div", "panel");
  panneau.innerHTML = `
    <div class="panel-head">
      <h3><span data-lang="fr">Rejouer une preuve dans la vraie cible</span><span data-lang="en">Replay a proof inside the real target</span></h3>
      <span id="cibleEtat" class="badge warn">?</span>
    </div>
    <p class="hint">
      <span data-lang="fr">Le service public (<code>banc/service.py</code>) exécute la commande <b>dans l'invité QNX 8.0</b> et renvoie sa sortie, comparée octet pour octet à celle du poste. La liste des étapes est <b>fermée</b> : le navigateur ne peut demander qu'un identifiant, jamais une commande libre.</span>
      <span data-lang="en">The public service (<code>banc/service.py</code>) runs the command <b>inside the QNX 8.0 guest</b> and returns its output, compared byte for byte with the workstation's. The step list is <b>closed</b>: the browser can only ask for an identifier, never a free-form command.</span>
    </p>
    <div class="row">
      <button id="cibleEtatBtn"><span data-lang="fr">État de l'invité</span><span data-lang="en">Guest status</span></button>
      <button id="cibleRun"><span data-lang="fr">Rejouer la preuve</span><span data-lang="en">Replay the proof</span></button>
      <select id="cibleEtape"></select>
      <span class="hint" id="cibleNote"></span>
    </div>
    <h4><span data-lang="fr">Invité QNX 8.0</span><span data-lang="en">QNX 8.0 guest</span></h4>
    <pre id="cibleInvite" class="output short"></pre>
    <h4><span data-lang="fr">Poste (hôte)</span><span data-lang="en">Workstation (host)</span></h4>
    <pre id="cibleHote" class="output short"></pre>
  `;
  const dernier = live.querySelector(".two");
  live.insertBefore(panneau, dernier ? dernier.nextSibling : null);

  const etatBadge = $("cibleEtat");
  const inviteOut = $("cibleInvite");
  const hoteOut = $("cibleHote");
  const note = $("cibleNote");
  const etapeSel = $("cibleEtape");

  function badge(texte, cls) {
    etatBadge.textContent = texte;
    etatBadge.className = "badge " + cls;
    etatBadge.hidden = false;
  }

  function rafraichirEtape() {
    return appeler("/etat", 20000).then((r) => {
      const st = r.corps && r.corps.invite ? r.corps.invite : {};
      const etapes = (r.corps && r.corps.preuves) || [];
      if (!etapeSel.options.length) {
        etapes.forEach((p) => {
          const o = document.createElement("option");
          o.value = p.id;
          o.textContent = lang() === "fr" ? p.titre_fr : p.titre_en;
          etapeSel.appendChild(o);
        });
      }
      badge(st.pret ? t("invité prêt", "guest ready") : (st.actif ? t("invité en démarrage", "guest booting") : t("invité arrêté", "guest stopped")),
        st.pret ? "ok" : "warn");
      return r;
    });
  }

  function afficher(corps) {
    inviteOut.textContent = corps.sortie_invite || corps.erreur || "";
    hoteOut.textContent = corps.sortie_hote || "";
    const idem = corps.identique_octet_pour_octet;
    if (idem === true) {
      badge(t("identique (octet pour octet)", "identical (byte for byte)"), "ok");
    } else if (idem === false) {
      badge(t("divergence", "divergence"), "ko");
    } else {
      badge(t("non comparable", "not comparable"), "warn");
    }
  }

  $("cibleEtatBtn").onclick = () => {
    note.textContent = t("interrogation…", "asking…");
    rafraichirEtape().then((r) => {
      note.textContent = r.ok ? "" : t("service injoignable", "service unreachable");
    });
  };

  $("cibleRun").onclick = () => {
    const id = etapeSel.value;
    if (!id) return;
    note.textContent = t("exécution dans l'invité (l'invité peut démarrer)…", "running in the guest (it may boot)…");
    badge(t("en cours", "running"), "warn");
    appeler("/preuve/" + encodeURIComponent(id), DELAI).then((r) => {
      if (!r.ok && r.corps && r.corps.erreur) {
        note.textContent = t("échec : ", "failed: ") + r.corps.erreur;
        badge(t("échec", "failed"), "ko");
        return;
      }
      note.textContent = "";
      afficher(r.corps);
    });
  };

  rafraichirEtape();
})();
