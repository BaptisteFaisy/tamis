#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pont local Tamis — le site public pilote le VRAI cœur C et le VRAI banc QNX.

Le site (https://tradoooo.github.io/tamis/) est statique : il ne peut héberger ni le
binaire C, ni un hyperviseur, ni l'image QNX (6 Gio, non redistribuable). Ce pont comble
l'écart : il tourne sur TA machine, le site s'y connecte quand tu le lui demandes.

Sécurité cumulative :
  1. écoute sur **127.0.0.1 uniquement** — jamais joignable depuis le réseau ;
  2. **jeton aléatoire** imprimé au démarrage, exigé dans l'en-tête `X-Tamis-Token` ;
  3. en-tête **Origin contrôlé** (site Tamis ou localhost) ;
  4. commandes sur **liste blanche**, aucun shell (argv), corps de requête borné.
Sans le jeton, un site tiers ne peut rien faire, même s'il devine le port.

Usage : python3 tools/tamis-bridge.py [--port 8787] [--vm DOSSIER] [--token-fixe JETON]
"""
from __future__ import annotations

import argparse
import json
import os
import secrets
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bridge_backend as B  # noqa: E402

VERSION = "0.1.0"
MAX_BODY = 1024 * 1024
SITE = "https://tradoooo.github.io"


def origin_ok(origin: str | None) -> bool:
    """Origine admise : le site Tamis, ou un hôte local (tests, copie locale du site)."""
    if origin is None:
        return True  # requête sans Origin (curl, test) — le jeton reste exigé
    if origin == SITE:
        return True
    return origin.startswith(("http://localhost:", "http://127.0.0.1:"))


class Pont(BaseHTTPRequestHandler):
    server_version = "TamisBridge/" + VERSION
    protocol_version = "HTTP/1.1"
    token = ""

    def log_message(self, fmt, *args):
        sys.stderr.write("· %s %s\n" % (self.command, fmt % args))

    # ——— réponses ———
    def _cors(self):
        origin = self.headers.get("Origin")
        self.send_header("Access-Control-Allow-Origin", origin if origin_ok(origin) else "*")
        self.send_header("Access-Control-Allow-Headers", "Content-Type, X-Tamis-Token")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Private-Network", "true")
        self.send_header("Access-Control-Max-Age", "600")
        self.send_header("Vary", "Origin")

    def _json(self, code: int, obj: dict):
        corps = json.dumps(obj, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
        self.send_response(code)
        self._cors()
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(corps)))
        self.end_headers()
        self.wfile.write(corps)

    def _body(self) -> dict:
        n = int(self.headers.get("Content-Length") or 0)
        if n <= 0:
            return {}
        if n > MAX_BODY:
            raise ValueError("corps trop volumineux")
        return json.loads(self.rfile.read(n).decode("utf-8"))

    def _garde(self) -> bool:
        """Origin + jeton. Renvoie True si la requête peut continuer."""
        origin = self.headers.get("Origin")
        if not origin_ok(origin):
            self._json(403, {"error": "Origine refusée", "origin": origin})
            return False
        if self.path != "/ping" and self.headers.get("X-Tamis-Token") != self.token:
            self._json(401, {"error": "jeton absent ou invalide (en-tête X-Tamis-Token)"})
            return False
        return True

    # ——— verbes ———
    def do_OPTIONS(self):
        self.send_response(204)
        self._cors()
        self.send_header("Content-Length", "0")
        self.end_headers()

    def do_GET(self):
        if not self._garde():
            return
        if self.path == "/ping":
            self._json(200, {"ok": True, "service": "tamis-bridge", "version": VERSION,
                             "cli_c": B.CLI.is_file(), "qnx": B.qnx_status()})
        elif self.path == "/qnx/status":
            self._json(200, B.qnx_status())
        elif self.path == "/presets":
            self._json(200, {"presets": [{"id": k, "label": v[0]}
                                         for k, v in B.PRESETS.items()]})
        else:
            self._json(404, {"error": "chemin inconnu", "path": self.path})

    def do_POST(self):
        if not self._garde():
            return
        try:
            corps = self._body()
        except (ValueError, json.JSONDecodeError) as exc:
            self._json(400, {"error": f"corps invalide : {exc}"})
            return
        self._route(self.path, corps)

    def _route(self, chemin: str, corps: dict):
        if chemin == "/tamis":
            self._json(200, B.run_tamis(
                corps.get("cmd", ""), corps.get("policy", ""), corps.get("policy2"),
                corps.get("src"), corps.get("dst"), corps.get("format", "text"),
                corps.get("label") or "policy.mpf"))
        elif chemin == "/qnx/boot":
            self._json(200, B.qnx_boot(int(corps.get("wait_s", 180))))
        elif chemin == "/qnx/stop":
            self._json(200, B.qnx_stop())
        elif chemin == "/qnx/exec":
            self._json(200, B.qnx_exec(corps.get("preset", "")))
        else:
            self._json(404, {"error": "chemin inconnu", "path": chemin})


def main(argv: list) -> int:
    ap = argparse.ArgumentParser(description="Pont local Tamis (127.0.0.1 uniquement)")
    ap.add_argument("--port", type=int, default=8787)
    ap.add_argument("--vm", help="dossier du banc QNX (contenant qnx-vm.sh)")
    ap.add_argument("--token-fixe", dest="token", help="jeton imposé (tests) ; défaut : aléatoire")
    a = ap.parse_args(argv[1:])
    if a.vm:
        os.environ["TAMIS_QNX_VM"] = a.vm

    Pont.token = a.token or secrets.token_urlsafe(18)
    srv = ThreadingHTTPServer(("127.0.0.1", a.port), Pont)
    qnx = B.qnx_status()
    print("=" * 68, flush=True)
    print("Pont local Tamis — n'écoute QUE sur 127.0.0.1 (jamais sur le réseau)", flush=True)
    print(f"  URL     : http://127.0.0.1:{a.port}", flush=True)
    print(f"  JETON   : {Pont.token}", flush=True)
    print(f"  cœur C  : {'présent' if B.CLI.is_file() else 'absent (make cli au premier appel)'}", flush=True)
    print(f"  banc QNX: {B.banc_label()} — " + (
        ("en marche" if qnx.get("running") else "arrêté") if qnx.get("configured")
        else "non configuré (TAMIS_QNX_VM ou TAMIS_QNX_HOST)"), flush=True)
    print("  Colle le jeton dans l'onglet « Banc réel » du site. Ctrl-C pour arrêter.")
    print("=" * 68, flush=True)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        print("\narrêt")
    finally:
        srv.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
