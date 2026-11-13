"use strict";
/* SPDX-License-Identifier: MIT */
/* Tamis — codelab interactif : rejoue chaque étape DANS le navigateur, avec le vrai
 * jumeau (le langage secpol QNX 8.0 inclus). Aucune logique dupliquée : le parse,
 * l'exécution et le rendu passent tous par site/tamis-run.js, comme le playground.
 * window.TamisCodelab (exposé pour le test sans navigateur). */

(function () {
  const Run = window.TamisRun;
  if (!Run) return;
  const EX = window.TAMIS_EXAMPLES || {};

  function extension(policy) {
    return Run.detectFormat(EX[policy] || "") === "secpol" ? "secpol" : "mpf";
  }

  function metaDe(policy) {
    return {
      policy_file: policy + "." + extension(policy),
      policy_sha256: "non calculée dans le navigateur (outil hôte : tools/sha256.c)",
      timestamp_utc: new Date().toISOString().replace(/\.\d+Z$/, "Z"),
      tamis_version: "0.1.0 (jumeau navigateur)",
    };
  }

  /* exécute l'étape décrite par les data-* du bouton et écrit dans la cible. */
  function lancer(btn) {
    const cible = document.getElementById(btn.dataset.target);
    if (!cible) return null;
    const policy = btn.dataset.policy;
    const label = btn.dataset.label || (policy + "." + extension(policy));
    const r = Run.executer({
      cmd: btn.dataset.cmd,
      policy: EX[policy] || "",
      policy2: EX[btn.dataset.policy2] || "",
      src: btn.dataset.src || null,
      dst: btn.dataset.dst || null,
      label,
    });
    let texte;
    if (r.nonComparable) texte = "(commande non comparable dans le navigateur)";
    else if (r.vue) texte = r.vue;
    else texte = Run.rendre(btn.dataset.fmt || "text", r.verdicts || [], metaDe(policy));
    cible.hidden = false;
    cible.textContent = texte;
    return texte;
  }

  function brancher() {
    const boutons = document.querySelectorAll ? document.querySelectorAll("[data-codelab]") : [];
    for (let i = 0; i < boutons.length; i++) {
      boutons[i].addEventListener("click", function () { lancer(boutons[i]); });
    }
  }

  window.TamisCodelab = { lancer, brancher };
  /* En navigateur : brancher dès que le DOM est prêt. Sous Node (test), c'est le test qui appelle. */
  if (typeof document !== "undefined" && document.addEventListener) {
    if (document.readyState === "loading") document.addEventListener("DOMContentLoaded", brancher);
    else brancher();
  }
})();