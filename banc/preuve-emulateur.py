#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Preuves produites DANS l'invité QNX 8.0 (émulateur QEMU), comparées à l'hôte.

Pour chaque étape de `banc/commandes-emulateur.json` :
  1. la commande est exécutée dans l'invité QNX 8.0 (via banc/emulateur.py) ;
  2. la commande équivalente est exécutée sur l'hôte (le même cœur C, compilé pour POSIX) ;
  3. les deux sorties sont comparées OCTET POUR OCTET.

Le résultat est écrit dans `preuve/emulateur/evidences.json` : c'est la preuve que le
binaire QNX 8.0 (compilé par qcc) rend exactement les mêmes verdicts que le binaire hôte.

Usage : python3 banc/preuve-emulateur.py [--sortie <fichier.json>]
"""
from __future__ import annotations

import datetime
import hashlib
import json
import pathlib
import subprocess
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RACINE / "banc"))

from emulateur import Invite, Erreur  # noqa: E402
import invite_cycle  # noqa: E402

COMMANDES = RACINE / "banc" / "commandes-emulateur.json"
SORTIE = RACINE / "preuve" / "emulateur" / "evidences.json"


def sha(texte: str) -> str:
    return hashlib.sha256(texte.encode("utf-8")).hexdigest()


def sur_hote(argv: list[str]) -> tuple[str, int]:
    """Exécute la commande équivalente sur l'hôte et renvoie (sortie, code)."""
    proc = subprocess.run(argv, cwd=RACINE, capture_output=True, text=True)
    return (proc.stdout + proc.stderr).strip("\r\n"), proc.returncode


def identite(invite: Invite, conf: dict) -> dict:
    """Relève l'identité de l'invité (noyau, processus) : la cible est-elle bien QNX 8.0 ?"""
    releve: dict = {}
    for item in conf["identite"]:
        releve[item["id"]] = {
            "titre_fr": item["titre_fr"],
            "commande": " ".join(item["invite"]),
            "sortie": invite.executer(" ".join(item["invite"])),
        }
    return releve


def preuve(invite: Invite, item: dict) -> dict:
    """Exécute une étape dans l'invité, la même sur l'hôte, et compare les deux sorties."""
    invite_sortie = invite.executer(" ".join(item["invite"]))
    hote_sortie, code = sur_hote(item["hote"])
    identique = invite_sortie == hote_sortie
    if item.get("comparaison_octets") is False:
        identique = None       # différence attendue et documentée
    return {
        "id": item["id"],
        "titre_fr": item["titre_fr"],
        "titre_en": item["titre_en"],
        "commande_invite": " ".join(item["invite"]),
        "commande_hote": " ".join(item["hote"]),
        "sortie_invite": invite_sortie,
        "sortie_hote": hote_sortie,
        "code_hote": code,
        "identique_octet_pour_octet": identique,
        "sha256_invite": sha(invite_sortie),
        "sha256_hote": sha(hote_sortie),
        "note_fr": item.get("note_fr", ""),
        "note_en": item.get("note_en", ""),
    }


def resume(rapport: dict) -> dict:
    """Résumé chiffré du rapport (jamais de chiffre écrit à la main)."""
    identiques = [p for p in rapport["preuves"] if p["identique_octet_pour_octet"] is True]
    identites = rapport.get("identite", {})
    return {
        "preuves": len(rapport["preuves"]),
        "identiques_octet_pour_octet": len(identiques),
        "identite_invite": next(
            (v.get("sortie", "").splitlines()[0] for v in identites.values() if v.get("sortie")),
            "",
        ),
    }


def main(argv: list[str]) -> int:
    sortie = SORTIE
    if "--sortie" in argv:
        sortie = RACINE / argv[argv.index("--sortie") + 1]
    conf = json.loads(COMMANDES.read_text(encoding="utf-8"))
    rapport: dict = {
        "genere_le": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds"),
        "source": "banc/preuve-emulateur.py",
        "commande_source": "banc/commandes-emulateur.json",
    }
    invite = None
    try:
        invite_cycle.reveiller()          # l'invité ne tourne que si on le regarde
        invite = invite_cycle.attendre()
        if invite is None:
            print("KO émulateur : l'invité QNX 8.0 ne répond pas", file=sys.stderr)
            return 1
        rapport["identite"] = identite(invite, conf)
        rapport["preuves"] = [preuve(invite, item) for item in conf["preuves"]]
    except Erreur as e:
        print(f"KO émulateur : {e}", file=sys.stderr)
        return 1
    finally:
        if invite is not None:
            invite.fermer()

    rapport["resume"] = resume(rapport)
    sortie.parent.mkdir(parents=True, exist_ok=True)
    sortie.write_text(json.dumps(rapport, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    identiques = rapport["resume"]["identiques_octet_pour_octet"]
    print(f"OK {identiques}/{rapport['resume']['preuves']} preuves identiques octet pour octet")
    print(f"   artefact : {sortie.relative_to(RACINE)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
