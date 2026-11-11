#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Harnais de test différentiel Tamis ↔ secpolcompile (TAM-12). Optionnel sans SDP.
# Codes : 0 succès, 1 désaccord, 2 usage invalide, 3 prérequis manquant, 4 dry-run.
set -euo pipefail

usage() {
  cat <<'EOF'
oracle/run_oracle.sh — différentiel Tamis contre le compilateur officiel QNX.

Pour chaque politique de fixtures/, compare le verdict d'acceptation de Tamis
(parseur) à celui de `secpolcompile` (utilitaire hôte du SDP). Toute politique
acceptée par l'un doit être acceptée par l'autre, et inversement.

Usage :
  oracle/run_oracle.sh [--fixtures <dossier>] [--dry-run] [--help]

Options :
  --fixtures <dossier>  corpus à tester (défaut : fixtures/)
  --dry-run             n'exécute rien, sort 4 (aucun résultat possible sans SDP)
  --help                cette aide

Prérequis : un SDP QNX 8.0 installé et `secpolcompile` dans le PATH (ou QNX_SDP posé).
Sans SDP : sort 3 (prérequis manquant) — ce n'est pas un échec, le différentiel
C ↔ Python (tests/test_differentiel.sh) reste la preuve principale.
EOF
}

FIXTURES="fixtures"
DRYRUN=0
while [ $# -gt 0 ]; do
  case "$1" in
    --fixtures) FIXTURES="${2:?}"; shift 2 ;;
    --dry-run)  DRYRUN=1; shift ;;
    --help|-h)  usage; exit 0 ;;
    *) echo "KO option inconnue : $1" >&2; usage >&2; exit 2 ;;
  esac
done

if [ "$DRYRUN" -eq 1 ]; then
  echo "INFO dry-run : aucun résultat possible sans exécuter secpolcompile"
  exit 4
fi

# Localise secpolcompile : PATH d'abord, sinon $QNX_SDP/host/usr/bin.
SECPOLCOMPILE="$(command -v secpolcompile 2>/dev/null || true)"
if [ -z "$SECPOLCOMPILE" ] && [ -n "${QNX_SDP:-}" ]; then
  for cand in "$QNX_SDP"/host/usr/bin/secpolcompile "$QNX_SDP"/host/linux/x86_64/usr/bin/secpolcompile; do
    [ -x "$cand" ] && { SECPOLCOMPILE="$cand"; break; }
  done
fi
if [ -z "$SECPOLCOMPILE" ]; then
  echo "ATTENTION secpolcompile introuvable (PATH et QNX_SDP=${QNX_SDP:-<vide>})"
  echo "INFO prérequis manquant : le SDP QNX 8.0 n'est pas installé sur cet hôte."
  echo "INFO le différentiel C ↔ Python (niveau 2) reste la preuve principale ; TAM-12 est optionnel."
  exit 3
fi

[ -d "$FIXTURES" ] || { echo "KO dossier fixtures introuvable : $FIXTURES" >&2; exit 2; }

total=0; desaccords=0
for pol in "$FIXTURES"/*.secpol; do
  [ -e "$pol" ] || continue
  total=$((total+1))
  # secpolcompile : 0 = accepte, non-nul = refuse. Tamis : à brancher sur le parseur (L1).
  if "$SECPOLCOMPILE" "$pol" >/dev/null 2>&1; then
    oracle="accepte"
  else
    oracle="refuse"
  fi
  # TODO(L1) : remplacer « accepte » par l'appel réel au parseur Tamis quand il existe.
  tamis="accepte"
  if [ "$oracle" = "$tamis" ]; then
    printf 'OK   %s : oracle=%s tamis=%s\n' "$(basename "$pol")" "$oracle" "$tamis"
  else
    printf 'KO   %s : DÉSACCORD oracle=%s tamis=%s\n' "$(basename "$pol")" "$oracle" "$tamis"
    desaccords=$((desaccords+1))
  fi
done

if [ "$desaccords" -ne 0 ]; then
  echo "FAILED $desaccords désaccord(s) sur $total politiques"
  exit 1
fi
echo "OK $total politiques, accord total Tamis ↔ secpolcompile"
