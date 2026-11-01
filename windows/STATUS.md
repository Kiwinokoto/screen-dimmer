# Passation Windows — 2026-11-01

- Source GitHub : `Kiwinokoto/screen-dimmer`, `main` à `7d47418a598528efa231d716471ee1ab929bbf72` au début de la passe.
- Développement isolé : `dev/windows-overlay` ; Linux/X11 non modifié, hormis références documentaires.
- Implémentation : Win32/C++17, overlay noir click-through sur tous les moniteurs, switch et slider 10–100 %, tray, préférences INI, commandes pour raccourcis et PowerShell d'installation utilisateur.
- Contrôles locaux : tests de logique C++17 via `g++` OK ; `bash -n` et `tests/smoke.sh` Linux OK ; `git diff --check` OK.
- Non testé : compilation MSVC/Windows, UI, tray, écrans multiples, HDR et bascule après fermeture brutale. Aucun compilateur Windows disponible dans l'environnement local.
- `.agentctl.json` existe, mais aucun client AgentCTL utilisable ici : l'absence de bail concurrent n'a pas été vérifiée auprès du contrôleur central. Ne pas toucher à `main` avant cette vérification.
- Suite : compiler avec le workflow Windows sur la branche dédiée ; corriger erreurs éventuelles ; faire tester l'exécutable sur le HP Windows 11 ; seulement ensuite envisager merge / publication.
