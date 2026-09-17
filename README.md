# Screen Dimmer

Petit utilitaire Linux pour régler la luminosité **logicielle** de tous les écrans connectés via `xrandr`, avec une interface Zenity.

## Fonctionnalités

- interface graphique simple avec curseur de 10 à 100 % ;
- boutons **Appliquer**, **Annuler** et **Reset** ;
- commandes CLI pour augmenter, diminuer ou réinitialiser la luminosité ;
- mémorisation de la dernière valeur dans `~/.local/state/screen-dimmer/value` ;
- application de la même valeur à tous les écrans détectés par `xrandr`.

## Prérequis

- Linux sous session X11 ;
- Bash ;
- `xrandr` ;
- `zenity` pour l'interface graphique ;
- `awk`.

> Le réglage est logiciel : il ne modifie pas le rétroéclairage matériel de l'écran. Sous Wayland, `xrandr` peut ne pas fonctionner comme attendu.

## Installation

```bash
mkdir -p ~/.local/bin ~/.local/share/applications
cp screen-dimmer ~/.local/bin/screen-dimmer
chmod +x ~/.local/bin/screen-dimmer
cp screen-dimmer.desktop ~/.local/share/applications/screen-dimmer.desktop
```

Vérifier que `~/.local/bin` est présent dans le `PATH`, puis lancer :

```bash
screen-dimmer
```

Le fichier `.desktop` permet aussi de lancer Screen Dimmer depuis le menu des applications. Il peut être copié sur le Bureau si souhaité.

## Utilisation CLI

```bash
screen-dimmer          # ouvre l'interface
screen-dimmer ui       # idem
screen-dimmer up       # +5 %
screen-dimmer down     # -5 %
screen-dimmer reset    # remet à 100 %
screen-dimmer off      # alias de reset
screen-dimmer 0.40     # applique directement 40 %
```

Les valeurs directes sont bornées entre 10 % et 100 %.

## Configuration

Les constantes en tête du script contrôlent le comportement :

```bash
MIN="0.10"
MAX="1.00"
STEP="0.05"
DEFAULT="0.55"
```

`DEFAULT` est utilisé lorsqu'aucune valeur précédente n'a encore été enregistrée.

## Sauvegarde

Ce dépôt sert de source de sauvegarde du petit utilitaire installé localement. Le script `screen-dimmer` correspond à la version utilisée sur la machine Linux au moment de l'initialisation du dépôt, avec le bouton **Reset** ajouté à l'interface.
