#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# run_oracle.sh — différentiel oracle : le parseur de Tamis contre le compilateur officiel
# `secpolcompile` (TAM-12). Pour chaque politique du corpus, on compare l'ACCEPTATION :
#   secpolcompile : « 0 errors »  -> accepte ; sinon refuse
#   Tamis         : absence de TAMIS_PARSE_ERROR -> accepte ; sinon refuse
# Un désaccord est un BUG de Tamis (le compilateur fait autorité).
#
# Le compilateur écrit ses erreurs sur stdout et sort 0 même en cas d'échec : c'est le
# nombre de « N errors » qui fait foi, pas le code de retour.
#
# Codes : 0 accord total ; 1 désaccord ; 2 usage ; 3 prérequis manquant (pas de SDP).
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
SDP="${QNX_SDP:-$HOME/qnx800}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

usage() {
  cat <<'EOF'
usage : oracle/run_oracle.sh [--corpus <dossier>] [--fichier <policy.secpol>] [--aide]

Compare l'acceptation par Tamis et par secpolcompile, politique par politique.
Nécessite un SDP QNX installé (QNX_SDP ou ~/qnx800) et le CLI (build/tamis).
EOF
}

CORPUS=""
FICHIER=""
while [ $# -gt 0 ]; do
  case "$1" in
    --corpus) CORPUS="${2:?}"; shift 2 ;;
    --fichier) FICHIER="${2:?}"; shift 2 ;;
    --aide|-h) usage; exit 0 ;;
    *) echo "KO option inconnue : $1" >&2; usage >&2; exit 2 ;;
  esac
done

# ——— environnement du SDP ———
# `secpolcompile` ne s'exécute pas sans QNX_HOST/QNX_TARGET/QNX_CONFIGURATION (sort 127 sinon).
# On source l'env fourni par le SDP quand il existe, sans écraser celui de l'appelant.
if [ -z "${QNX_HOST:-}" ] && [ -f "$SDP/qnxsdp-env.sh" ]; then
  set +u
  # shellcheck disable=SC1090
  . "$SDP/qnxsdp-env.sh" >/dev/null 2>&1 || true
  set -u
fi

# ——— le compilateur officiel ———
SECPOLCOMPILE=""
for c in "$SDP/host/linux/x86_64/usr/bin/secpolcompile" "$SDP/host/usr/bin/secpolcompile"; do
  [ -n "$c" ] && [ -x "$c" ] && { SECPOLCOMPILE="$c"; break; }
done
if [ -z "$SECPOLCOMPILE" ] && command -v secpolcompile >/dev/null 2>&1; then
  SECPOLCOMPILE="$(command -v secpolcompile)"
fi
if [ -z "$SECPOLCOMPILE" ]; then
  echo "KO secpolcompile introuvable (QNX_SDP ou ~/qnx800 manquant) — prérequis manquant"
  exit 3
fi

# ——— le CLI de Tamis ———
if [ ! -x build/tamis ]; then
  make -s cli >/dev/null 2>&1 || { echo "KO impossible de construire build/tamis"; exit 3; }
fi

# ——— corpus ———
if [ -n "$FICHIER" ]; then
  LISTE="$FICHIER"
elif [ -n "$CORPUS" ]; then
  LISTE=$(find "$CORPUS" -name '*.secpol' -type f | sort)
else
  LISTE=$(find fixtures -name '*.secpol' -type f 2>/dev/null | sort)
fi
if [ -z "$LISTE" ]; then
  echo "KO aucune politique .secpol dans le corpus"
  exit 3
fi

total=0; accords=0; desaccords=0
printf '%-44s %-12s %-12s %s\n' "POLITIQUE" "secpolcompile" "tamis" "VERDICT"
printf '%-44s %-12s %-12s %s\n' "----------" "-------------" "------------" "-------"
for pol in $LISTE; do
  [ -f "$pol" ] || continue
  total=$((total+1))
  nom=$(basename "$pol")
  # Oracle : le compilateur écrit ses messages sur stdout et sort 0 SI ET SEULEMENT
  # S'IL N'Y A PAS D'ERREUR (et n'affiche rien quand tout va bien) : c'est le CODE DE
  # SORTIE qui fait foi, pas le texte « N errors » (qui n'apparaît que s'il y a un message).
  "$SECPOLCOMPILE" -o "$TMP/p.bin" "$pol" >"$TMP/msg.txt" 2>&1
  rc_o=$?
  if [ "$rc_o" -eq 126 ] || [ "$rc_o" -eq 127 ]; then
    echo "KO secpolcompile ne peut pas s'exécuter (rc=$rc_o) — prérequis d'environnement manquant"
    echo "   essayez : source \"$SDP/qnxsdp-env.sh\" puis relancez"
    exit 3
  fi
  if [ "$rc_o" -eq 0 ]; then
    oracle="accepte"
  else
    oracle="refuse"
  fi
  sortie=$(cat "$TMP/msg.txt")
  sortie_t=$(./build/tamis lint "$pol" --format json 2>&1)
  if printf '%s' "$sortie_t" | grep -q 'TAMIS_PARSE_ERROR'; then tamis="refuse"; else tamis="accepte"; fi
  if [ "$oracle" = "$tamis" ]; then
    accords=$((accords+1)); verdict="ACCORD"
  else
    desaccords=$((desaccords+1)); verdict="DÉSACCORD"
  fi
  printf '%-44s %-12s %-12s %s\n' "$nom" "$oracle" "$tamis" "$verdict"
  if [ "$verdict" = "DÉSACCORD" ]; then
    echo "      oracle : $(printf '%s' "$sortie" | grep -E 'error' | head -2 | tr '\n' ' ')"
    echo "      tamis  : $(printf '%s' "$sortie_t" | head -2 | tr '\n' ' ')"
  fi
done

echo
echo "$accords/$total politiques en accord avec le compilateur officiel"
if [ "$desaccords" -ne 0 ]; then
  echo "FAILED $desaccords désaccord(s) — le parseur de Tamis s'écarte de secpolcompile"
  exit 1
fi
echo "OK $total checks — Tamis et secpolcompile sont d'accord sur tout le corpus"
exit 0
