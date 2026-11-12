#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Tire une image Docker/OCI depuis le registry par l'API HTTP — sans docker, sans sudo.

Usage : pull-oci.py <repo> <tag> <dossier-sortie>
Exemple : pull-oci.py autosarai/qemu_qnx7_1 1.1.0 ~/qnx71-vm/dl

Étapes : jeton anonyme → manifeste (liste ou simple) → blobs des couches → extraction tar.
Robustesse : le jeton Docker Hub expire (~5 min) — il est rafraîchi sur 401. Chaque couche
extraite laisse un marqueur `.ok-<i>` : un relancement reprend où il s'est arrêté.
"""
from __future__ import annotations

import json
import subprocess
import sys
import urllib.error
import urllib.request
from pathlib import Path

REG = "https://registry-1.docker.io"
AUTH = "https://auth.docker.io/token"
ACCEPT = ", ".join([
    "application/vnd.docker.distribution.manifest.v2+json",
    "application/vnd.oci.image.manifest.v1+json",
    "application/vnd.docker.distribution.manifest.list.v2+json",
    "application/vnd.oci.image.index.v1+json",
])

_JETON: dict = {"repo": None, "val": None}


def log(msg: str) -> None:
    print(msg, flush=True)


def jeton(repo: str, force: bool = False) -> str:
    if force or _JETON["val"] is None or _JETON["repo"] != repo:
        url = f"{AUTH}?service=registry.docker.io&scope=repository:{repo}:pull"
        with urllib.request.urlopen(url, timeout=60) as r:
            _JETON["val"] = json.load(r)["token"]
            _JETON["repo"] = repo
    return _JETON["val"]


def fetch(url: str, repo: str, dest: Path | None = None):
    """GET avec rafraîchissement du jeton sur 401 (une seule nouvelle tentative)."""
    for essai in (0, 1):
        req = urllib.request.Request(
            url, headers={"Authorization": f"Bearer {jeton(repo, force=essai == 1)}",
                          "Accept": ACCEPT})
        try:
            with urllib.request.urlopen(req, timeout=900) as r:
                if dest is None:
                    return r.read()
                ecrit = 0
                with open(dest, "wb") as f:
                    while True:
                        bloc = r.read(1 << 20)
                        if not bloc:
                            break
                        f.write(bloc)
                        ecrit += len(bloc)
                return ecrit
        except urllib.error.HTTPError as e:
            if e.code == 401 and essai == 0:
                log("  · jeton expiré, renouvellement…")
                continue
            raise
    raise SystemExit("KO authentification registry")


def manifeste(repo: str, tag: str) -> dict:
    data = json.loads(fetch(f"{REG}/v2/{repo}/manifests/{tag}", repo))
    if "manifests" in data:
        for m in data["manifests"]:
            p = m.get("platform", {})
            if p.get("os") == "linux" and p.get("architecture") == "amd64":
                log(f"· index → variante amd64 {m['digest'][:19]}")
                return json.loads(fetch(f"{REG}/v2/{repo}/manifests/{m['digest']}", repo))
        raise SystemExit("KO aucune variante linux/amd64 dans l'index")
    return data


def extraire(blob: Path, cible: Path) -> list:
    """Extrait une couche avec le `tar` système (pas tarfile de Python).

    Les couches Docker contiennent des liens symboliques vers des chemins absolus
    (ex. etc/alternatives/awk -> /usr/bin/mawk) que les filtres Python refusent.
    GNU tar les crée tels quels (un lien n'écrit rien hors destination) et retire le
    « / » initial des chemins de fichiers réguliers.
    """
    liste = subprocess.run(["tar", "-tf", str(blob)], capture_output=True, text=True, check=True)
    r = subprocess.run(["tar", "-xf", str(blob), "-C", str(cible)], capture_output=True, text=True)
    if r.returncode != 0:
        raise SystemExit(f"KO extraction de {blob.name} : {r.stderr[:400]}")
    return [n for n in liste.stdout.splitlines() if n and not n.endswith("/")]


def main(argv: list) -> int:
    if len(argv) != 4:
        print(__doc__)
        return 2
    repo, tag, sortie = argv[1], argv[2], Path(argv[3]).expanduser()
    sortie.mkdir(parents=True, exist_ok=True)
    log(f"OK dépôt {repo}:{tag} → {sortie}")
    mf = manifeste(repo, tag)
    couches = mf.get("layers", [])
    log(f"· {len(couches)} couche(s) à tirer")
    tous = []
    for i, c in enumerate(couches, 1):
        marqueur = sortie / f".ok-{i}"
        if marqueur.exists():
            log(f"· couche {i}/{len(couches)} déjà extraite, ignorée")
            continue
        blob = sortie / f"layer-{i}.blob"
        log(f"· couche {i}/{len(couches)} {c['digest'][:19]} ({c.get('size', 0)/1e6:.1f} Mo)…")
        n = fetch(f"{REG}/v2/{repo}/blobs/{c['digest']}", repo, blob)
        log(f"  téléchargé {n/1e6:.1f} Mo")
        tous += extraire(blob, sortie)
        blob.unlink(missing_ok=True)
        marqueur.write_text(c["digest"], encoding="utf-8")
    log(f"OK extraction terminée ({len(tous)} fichiers ce run)")
    gros = sorted((p for p in sortie.rglob("*") if p.is_file() and not p.name.startswith(".ok-")),
                  key=lambda p: p.stat().st_size, reverse=True)[:12]
    log("· plus gros fichiers :")
    for p in gros:
        log(f"    {p.stat().st_size/1e9:8.3f} Go  {p.relative_to(sortie)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
