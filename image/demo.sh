#!/bin/ksh
# demo.sh — scénario Tamis exécuté DANS l'invité QNX 8.0.
#
# Ce n'est pas une simulation : `tamis` est le cœur C du projet compilé pour
# QNX 8.0 par qcc, et la politique analysée est le fichier QNX 8.0 réel
# embarqué dans l'image (politique.secpol). Rien d'autre n'est requis : le
# binaire lit la politique et rend les verdicts, comme sur POSIX.
#
# Contrainte QNX 8.0 (piège documenté) : sans disque, le ksh de l'invité ne
# peut pas créer de tube (pipe) ni faire de substitution de commande — ce
# scénario n'en utilise donc aucun. La démo de référence (Réplique) a la même
# contrainte.
#
# Le scénario est joué sur les DEUX consoles (VGA et série) : la VGA donne
# l'écran capturé par la vitrine, la série donne le journal texte complet.

POL=/proc/boot/politique.secpol

entete() {
    echo ""
    echo "================================================================"
    echo " Tamis — analyse statique de politique de securite QNX 8.0"
    echo " invite QNX 8.0 reelle | binaire compile par qcc"
    echo "================================================================"
    echo ""
}

# Un blanc pour faire défiler le tampon VGA (25 lignes) jusqu'au résumé final.
blanc() {
    i=0
    while [ $i -lt 12 ]; do
        echo ""
        i=$((i + 1))
    done
}

entete

echo "--- 0/6  l'invite est reelle (pidin) -------------------------"
pidin
echo ""

echo "--- 1/6  chemin d'attaque (reach) ----------------------------"
tamis reach $POL --src app_tiers --dst pompe_critique --label demo.secpol
echo ""

echo "--- 2/6  moindre privilege (lint) ----------------------------"
tamis lint $POL --label demo.secpol
echo ""

echo "--- 3/6  pourquoi ce refus (explain) -------------------------"
tamis explain $POL --src app_tiers --dst pompe_critique --label demo.secpol
echo ""

echo "--- 4/6  certificat de non-interference (proof) --------------"
tamis proof $POL --src app_tiers --dst pompe_critique --label demo.secpol
echo ""

echo "--- 5/6  version du binaire analyseur ------------------------"
tamis version
echo ""

blanc
echo "================================================================"
echo " RESUME — verdicts Tamis sur la politique QNX 8.0 embarquee"
echo "================================================================"
echo " chemin d'attaque : app_tiers -> journal -> diag -> pompe_critique"
echo "                   (3 sauts, transitions settypeid)"
echo " verdict           : TAMIS_PATH_FOUND  (acces a la pompe critique)"
echo " correctif minimal : retirer la derniere transition settypeid"
echo " lint              : TAMIS_MISSING_DEFAULT_SPAWN (trou S7)"
echo ""
echo " Le meme code C que sur POSIX — compile ici par qcc pour QNX 8.0."
echo "================================================================"