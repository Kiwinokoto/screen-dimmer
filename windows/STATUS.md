# Passation Windows — 2026-11-01

- Source GitHub : `Kiwinokoto/screen-dimmer`, `main` à `7d47418a598528efa231d716471ee1ab929bbf72` au début de la passe.
- Développement isolé : `dev/windows-overlay` ; Linux/X11 non modifié, hormis références documentaires.
- Implémentation : Win32/C++17, overlay noir click-through sur tous les moniteurs, switch et slider 10–100 %, tray, préférences INI, commandes pour raccourcis et PowerShell d'installation utilisateur.
- Contrôles locaux : tests de logique C++17 via `g++` OK ; `bash -n` et `tests/smoke.sh` Linux OK ; `git diff --check` OK.
- Compilation Windows : **OK** (GitHub Actions `41343081669`, commit source `004faadd`, MSVC 19.51, 2026-11-01), liaison statique du runtime C++ activée ; CTest Windows **1/1 réussi**. Artefact `ScreenDimmer-Windows` ID `13098311419` disponible jusqu'au 2026-11-15.
- Non testé : comportement visuel réel de l'UI, tray, écrans multiples, HDR et bascule après fermeture brutale. Aucune session Windows interactive disponible dans cet environnement.
- `.agentctl.json` existe, mais aucun client AgentCTL utilisable ici : l'absence de bail concurrent n'a pas été vérifiée auprès du contrôleur central. Ne pas toucher à `main` avant cette vérification.
- Suite : faire tester l'exécutable sur le HP Windows 11 ; corriger les éventuels défauts ; seulement ensuite envisager merge / publication. Ne pas lancer manuellement un workflow sans demande explicite.
