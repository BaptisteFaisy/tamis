#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Service HTTP : exécuter les étapes de preuve DANS l'invité QNX 8.0 (émulateur).

Le site (playground, codelab, onglet Preuves) appelle ce service pour que la démo et les
preuves soient produites par une **vraie cible QNX**, et non par le jumeau du navigateur.

Sécurité : la surface est une **liste fermée** (`banc/commandes-emulateur.json`). Aucune
commande libre n'est acceptée : le client ne peut demander qu'un identifiant d'étape.
Le service n'écoute que sur 127.0.0.1 (l'exposition publique se fait par tailscale funnel).

Cycle de vie : l'invité QNX 8.0 ne tourne que quand quelqu'un le regarde (la vitrine
l'arrête après quelques minutes d'inactivité). Le service réveille donc l'invité au
moment de servir une preuve — voir `banc/invite-cycle.py`.

Routes :
    GET /etat        → état de l'invité + liste des étapes disponibles
    GET /preuve/<id> → exécute l'étape dans l'invité et renvoie sa sortie (et celle de l'hôte)
"""
from __future__ import annotations

import json
import os
import pathlib
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

RACINE = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RACINE / "banc"))

import invite_cycle  # noqa: E402
from emulateur import Invite, Erreur  # noqa: E402

PORT = int(os.environ.get("TAMIS_EMUL_PORT", "8790"))
HOTE = os.environ.get("TAMIS_EMUL_HOTE", "127.0.0.1")
ORIGINES = os.environ.get(
    "TAMIS_EMUL_ORIGIN",
    "https://tradoooo.github.io,https://baptistefaisy.github.io,http://127.0.0.1:8790,http://localhost:8790",
).split(",")
COMMANDES = RACINE / "banc" / "commandes-emulateur.json"
VERROU = threading.Lock()          # le moniteur QEMU n'accepte qu'une connexion à la fois
DELAI_PREUVE = float(os.environ.get("TAMIS_EMUL_DELAI", "75"))


def config() -> dict:
    return json.loads(COMMANDES.read_text(encoding="utf-8"))


def sur_hote(argv: list[str]) -> tuple[str, int]:
    proc = subprocess.run(argv, cwd=RACINE, capture_output=True, text=True)
    return (proc.stdout + proc.stderr).strip("\r\n"), proc.returncode


def etape_de(conf: dict, ident: str) -> dict | None:
    """Retrouve l'étape demandée dans la liste fermée."""
    return next((p for p in conf["preuves"] if p["id"] == ident), None)


def executer_dans_invite(etape: dict) -> tuple[str, bool]:
    """Exécute l'étape dans l'invité (réveillé au besoin). Rend (sortie, succès)."""
    invite_cycle.reveiller()
    invite = invite_cycle.attendre(DELAI_PREUVE)
    if invite is None:
        return "invité QNX 8.0 indisponible (démarrage ou moniteur injoignable)", False
    try:
        return invite.executer(" ".join(etape["invite"])), True
    except Erreur as e:
        return f"invité injoignable : {e}", False
    finally:
        invite.fermer()


class Gestionnaire(BaseHTTPRequestHandler):
    server_version = "TamisEmulateur/1.0"

    def log_message(self, fmt, *args):        # journal discret
        pass

    def _entetes(self, code: int, type_p: str) -> None:
        self.send_response(code)
        self.send_header("Content-Type", type_p)
        origine = self.headers.get("Origin", "")
        if origine and any(origine == o.strip() for o in ORIGINES):
            self.send_header("Access-Control-Allow-Origin", origine)
            self.send_header("Vary", "Origin")
        self.send_header("Access-Control-Allow-Methods", "GET, OPTIONS")
        self.send_header("Cache-Control", "no-store")
        self.end_headers()

    def _json(self, code: int, obj: dict) -> None:
        corps = json.dumps(obj, ensure_ascii=False).encode("utf-8")
        self._entetes(code, "application/json; charset=utf-8")
        self.wfile.write(corps)

    def do_OPTIONS(self):                      # noqa: N802
        self._entetes(204, "text/plain")

    def _route_etat(self, conf: dict) -> None:
        etat_invite = invite_cycle.etat()
        self._json(200, {
            "invite": etat_invite,
            "preuves": [
                {"id": p["id"], "titre_fr": p["titre_fr"], "titre_en": p["titre_en"]}
                for p in conf["preuves"]
            ],
        })

    def _route_preuve(self, conf: dict, ident: str) -> None:
        etape = etape_de(conf, ident)
        if etape is None:
            self._json(404, {"erreur": "étape inconnue", "id": ident})
            return
        with VERROU:
            sortie_invite, invite_ok = executer_dans_invite(etape)
        sortie_hote, _ = sur_hote(etape["hote"])
        identique = sortie_invite == sortie_hote if invite_ok else None
        if etape.get("comparaison_octets") is False:
            identique = None
        self._json(200, {
            "id": etape["id"],
            "titre_fr": etape["titre_fr"],
            "titre_en": etape["titre_en"],
            "commande": " ".join(etape["invite"]),
            "sortie_invite": sortie_invite,
            "sortie_hote": sortie_hote,
            "identique_octet_pour_octet": identique,
            "note_fr": etape.get("note_fr", ""),
            "note_en": etape.get("note_en", ""),
        })

    def do_GET(self):                          # noqa: N802
        conf = config()
        if self.path == "/etat":
            self._route_etat(conf)
            return
        if self.path.startswith("/preuve/"):
            self._route_preuve(conf, self.path[len("/preuve/"):].strip("/"))
            return
        self._json(404, {"erreur": "chemin inconnu", "chemin": self.path})


def main() -> int:
    serveur = ThreadingHTTPServer((HOTE, PORT), Gestionnaire)
    print(f"Service émulateur Tamis sur http://{HOTE}:{PORT} (invité QNX 8.0)", flush=True)
    try:
        serveur.serve_forever()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
