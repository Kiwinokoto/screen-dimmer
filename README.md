# Screen Dimmer

Petit utilitaire Linux pour régler la luminosité **logicielle** de tous les écrans connectés via `xrandr`, avec une interface Zenity.

## Fonctionnalités

- interface graphique persistante avec curseur de 10 à 100 % ;
- application **en direct** pendant le déplacement du curseur, sans bouton Appliquer ;
- bouton **Reset** qui remet à 100 % et laisse l'interface disponible ;
- reset automatique à 100 % quand l'interface est fermée ou terminée, sans oublier le dernier réglage choisi ;
- garde-fou au login qui remet le multiplicateur logiciel `xrandr` à `1.00` après un arrêt brutal, sans écraser la préférence mémorisée ;
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
chmod +x install.sh
./install.sh
```

L'installateur reste entièrement dans le compte utilisateur : il place l'exécutable dans
`~/.local/bin`, le lanceur dans `~/.local/share/applications` et le garde-fou de login dans
`~/.config/autostart/screen-dimmer-reset.desktop`. Aucun `sudo` n'est utilisé.

Le garde-fou de login exécute `screen-dimmer startup-reset` et remet `xrandr` à `1.00`.
Cette valeur est le multiplicateur logiciel neutre : elle ne force pas la luminosité matérielle
du panneau. Le reset de fermeture et le reset de login ne remplacent pas la dernière valeur
choisie ; la prochaine ouverture retrouve donc la position précédente du curseur.

## Utilisation CLI

```bash
screen-dimmer          # ouvre l'interface
screen-dimmer ui       # idem
screen-dimmer up       # +5 %
screen-dimmer down     # -5 %
screen-dimmer reset    # remet à 100 %
screen-dimmer off      # alias de reset
screen-dimmer startup-reset  # reset silencieux utilisé au login
screen-dimmer 0.40     # applique directement 40 %
```

Les valeurs directes sont bornées entre 10 % et 100 %.

## Configuration

Les constantes en tête du script contrôlent le comportement :

```bash
MIN="0.10"
MAX="1.00"
STEP="0.05"
DEFAULT="1.00"
SYSTEM_DEFAULT="1.00"
```

`DEFAULT` est utilisé lorsqu'aucune valeur précédente n'a encore été enregistrée.
`SYSTEM_DEFAULT` est la valeur sûre restaurée à la fermeture et au login.

## Développement

Le dépôt est la source de vérité de l'installation locale. Après modification :

```bash
bash -n screen-dimmer install.sh tests/smoke.sh
./tests/smoke.sh
git diff --check
```

Installer uniquement après ces contrôles avec `./install.sh`.
