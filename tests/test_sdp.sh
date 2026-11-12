#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Test de la chaîne SDP : extraction automatique de la grammaire depuis une page HTML
# synthétique (l'extracteur doit être validé SANS le SDP installé) + chemin « pas de SDP ».
set -uo pipefail
RACINE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RACINE"
checks=0; fail=0
ck() { checks=$((checks+1)); if [ "$1" -eq 0 ]; then echo "OK   $2"; else echo "KO   $2"; fail=1; fi; }
ckc() { checks=$((checks+1)); if printf '%s' "$1" | grep -qF "$2"; then echo "OK   $3"; else echo "KO   $3 (absent)"; fail=1; fi; }

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

# ── 1. page HTML synthétique qui ressemble à une page de doc QNX ──────────────
mkdir -p "$TMP/sdp/8.0.0/htm/en/topic/com.qnx.doc.security.secpol"
mkdir -p "$TMP/sdp/host/usr/bin"
DOC="$TMP/sdp/8.0.0/htm/en/topic/com.qnx.doc.security.secpol/secpol_language.html"
cat > "$DOC" <<'HTML'
<html><body>
<h1>Security policy language</h1>
<p>The following productions describe the language.</p>
<pre>
type &lt;type_name&gt; ;
attribute &lt;name&gt; { &lt;type_list&gt; } ;
range &lt;name&gt; ;
allow &lt;src&gt; &lt;dst&gt;:&lt;ability&gt; { &lt;ability_list&gt; } ;
allow &lt;src&gt; &lt;dst&gt;:channel connect ;
allow_attach &lt;type&gt; &lt;path&gt; [ &lt;ftype&gt; : ] ;
allow_link &lt;type&gt; &lt;path&gt; ;
default_spawn_type &lt;type&gt; ;
derive_type &lt;type&gt; &lt;base&gt; ;
mount_point &lt;uid&gt;:&lt;gid&gt; &lt;perms&gt; ;
</pre>
<p>Rules may carry the modifiers nonroot, root, lock, no-inherit, inherit, subrange,
allow and deny.</p>
</body></html>
HTML
printf '#!/bin/sh\nexit 0\n' > "$TMP/sdp/host/usr/bin/secpolcompile"
chmod +x "$TMP/sdp/host/usr/bin/secpolcompile"

# ── 2. l'extracteur trouve les productions et le vocabulaire ──────────────────
out=$(python3 tools/extract_grammar.py "$TMP/sdp/8.0.0/htm/en/topic/com.qnx.doc.security.secpol" --markdown 2>&1)
ckc "$out" "type <type_name> ;" "extraction : production « type »"
ckc "$out" "allow <src> <dst>:<ability>" "extraction : production « allow »"
ckc "$out" "allow_attach <type> <path>" "extraction : production « allow_attach »"
ckc "$out" "default_spawn_type <type> ;" "extraction : production « default_spawn_type »"
ckc "$out" "no-inherit" "vocabulaire : modificateur « no-inherit » détecté"
ckc "$out" "subrange" "vocabulaire : modificateur « subrange » détecté"
ckc "$out" "relire" "avertissement de relecture présent"

# ── 3. la page unique fonctionne aussi ─────────────────────────────────────────
out2=$(python3 tools/extract_grammar.py "$DOC" --json 2>&1)
ckc "$out2" '"page": "secpol_language.html"' "mode JSON : page identifiée"
ckc "$out2" '"productions"' "mode JSON : productions listées"

# ── 4. sdp.sh détecte un SDP (factice) et joue le scénario complet ─────────────
out3=$(QNX_SDP="$TMP/sdp" bash banc/sdp.sh 2>&1)
ckc "$out3" "secpolcompile" "sdp.sh : cherche les outils hôtes"
ckc "$out3" "secpol_language.html" "sdp.sh : localise la doc installée"
ckc "$out3" "brouillon de grammaire extrait" "sdp.sh : déclenche l'extraction"

# ── 5. sdp.sh sans SDP : message clair et code 3 ───────────────────────────────
out4=$(QNX_SDP="$TMP/inexistant" bash banc/sdp.sh 2>&1)
rc=$?
[ "$rc" -eq 3 ]; ck $? "sdp.sh sans SDP : code de sortie 3 (prérequis manquant)"
ckc "$out4" "myQNX" "sdp.sh sans SDP : indique la marche à suivre"
ckc "$out4" "prédit" "sdp.sh sans SDP : rappelle ce qui reste à faire"

if [ "$fail" -ne 0 ]; then
  echo "FAILED test_sdp"
  exit 1
fi
echo "OK $checks checks"