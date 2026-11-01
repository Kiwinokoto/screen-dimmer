# Screen Dimmer pour Windows 10/11

Version native Win32 (runtime C++ lié statiquement), sans pilote, sans élévation, sans service ni dépendance à installer. Le filtre est une fenêtre noire transparente qui laisse passer les clics ; il ne modifie **pas** le rétroéclairage matériel. Le code Linux/X11 reste indépendant.

## Utilisation

- **Clic gauche** sur l'icône près de l'horloge : ouvre les réglages.
- **Switch Activer / Désactiver** : affiche ou retire immédiatement le filtre.
- **Curseur 10–100 %** : change la luminosité apparente en direct et active le filtre si nécessaire.
- **Clic droit** sur l'icône : activation, réglages, quitter.
- **Fermer la fenêtre** de réglage : le programme reste actif près de l'horloge.
- **Quitter** : supprime les filtres de tous les écrans.
- **Double-clic sur le raccourci Bureau** créé par l'installateur : bascule normal ↔ atténué sans ouvrir le panneau.

Le réglage est commun à tous les moniteurs. Le programme recrée les filtres lors d'un changement de configuration d'affichage. La préférence (55 % au premier lancement) est enregistrée dans `%LOCALAPPDATA%\ScreenDimmer\settings.ini`, séparément de l'état activé/désactivé.

Au démarrage d'un **nouveau processus**, le filtre est volontairement désactivé, même s'il était actif avant l'arrêt : protection après fermeture brutale ou mise à jour. La valeur préférée est conservée.

## Installation personnelle (sans admin)

1. Télécharger le ZIP `ScreenDimmer-Windows` depuis les artefacts de compilation GitHub et l'extraire.
2. Pour essayer : double-cliquer sur `ScreenDimmer.exe` (fenêtre de réglage, icône près de l'horloge).
3. Pour obtenir un raccourci Bureau et un démarrage automatique : dans PowerShell, depuis le dossier extrait :

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\install.ps1
   ```

   Option `-NoAutostart` pour installer sans créer l'entrée de démarrage automatique. L'installation ne modifie aucun paramètre de luminosité Windows et ne nécessite aucun administrateur.

**Attention :** un exécutable non signé peut déclencher un avertissement SmartScreen. Ne télécharger que depuis le dépôt officiel et ne pas désactiver globalement SmartScreen. Aucun pilote ni connexion réseau n'est utilisé par l'application.

## Commandes

`ScreenDimmer.exe` (ouvrir les réglages), `--toggle`, `--dim`, `--normal`, `--ui`, `--startup`, `--quit`.

Si une instance tourne déjà, les commandes sont envoyées à celle-ci. `--startup` désactive le filtre sans effacer la préférence et laisse le programme actif dans la zone de notification.

## Compilation depuis les sources

Visual Studio 2022 avec outils C++ et CMake :

```powershell
cmake -S windows -B build/windows -A x64
cmake --build build/windows --config Release
ctest --test-dir build/windows -C Release --output-on-failure
```

L'exécutable est dans `build/windows/Release/ScreenDimmer.exe`. Le test de logique pure peut aussi être compilé sous Linux avec un compilateur C++17 ; la compilation de l'interface et les tests d'affichage réels nécessitent Windows.

### Limites connues à tester

- Jeux ou vidéos en plein écran exclusif : l'overlay peut être masqué par l'application.
- Écran sécurisé de Windows/UAC : l'overlay n'y est pas affiché.
- HDR, changement d'échelle DPI, moniteurs branchés à chaud, bureau à distance : à vérifier sur machine réelle.
- Le filtre réduit la lumière **perçue**, pas la consommation du rétroéclairage.

Pour revenir immédiatement à la normale : désactiver le switch, cliquer droit sur l'icône puis **Quitter**, ou fermer le processus (les overlays disparaissent avec lui).
