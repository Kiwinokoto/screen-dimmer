# Screen Dimmer

Petit utilitaire Linux/X11 pour atténuer tous les écrans connectés via `xrandr`, avec un fonctionnement pensé comme un interrupteur simple.

## UX

- **Double-clic sur `Screen Dimmer` sur le Bureau** : bascule entre la luminosité logicielle normale (`xrandr --brightness 1.00`) et la valeur atténuée mémorisée.
- **Clic droit → Régler le mode nuit…** : action native Nemo, visible uniquement sur le raccourci Screen Dimmer du Bureau, qui ouvre le curseur Zenity et applique la valeur en direct.
- **Clic droit → Luminosité normale** : force immédiatement `1.00` sans effacer la valeur nocturne mémorisée.
- **Clic droit → Mode atténué** : réapplique immédiatement la valeur mémorisée.
- Fermer la fenêtre de réglage laisse la valeur choisie appliquée.
- Au login, un garde-fou remet seulement le multiplicateur logiciel à `1.00`, sans oublier la valeur atténuée.

Le réglage est logiciel : `1.00` est le multiplicateur neutre de `xrandr`. Screen Dimmer ne force pas la luminosité matérielle du rétroéclairage.

## Fonctionnalités

- toggle normal ↔ atténué ;
- curseur 10–100 % en direct, sans bouton Appliquer ;
- mémorisation séparée de la valeur atténuée et de l'état normal/atténué ;
- garde-fou de login après crash ou arrêt brutal ;
- même valeur appliquée à tous les écrans connectés ;
- commandes CLI ;
- raccourci Bureau et lanceur d'application générés par l'installateur.

## Prérequis

- Linux sous session X11 ;
- Bash ;
- `xrandr` ;
- `zenity` pour le curseur ;
- `awk`.

Sous Wayland, `xrandr` peut ne pas fonctionner comme attendu.

## Installation

```bash
chmod +x install.sh
./install.sh
```

Aucun `sudo` n'est utilisé. L'installateur crée ou met à jour :

- `~/.local/bin/screen-dimmer` ;
- `~/.local/share/applications/screen-dimmer.desktop` ;
- le raccourci `Screen Dimmer.desktop` dans le dossier Bureau réel renvoyé par `xdg-user-dir DESKTOP` ;
- `~/.local/share/nemo/actions/screen-dimmer-adjust.nemo_action`, qui fournit l’entrée de clic droit sur Cinnamon/Nemo ;
- `~/.config/autostart/screen-dimmer-reset.desktop`.

Le raccourci Bureau et son action de clic droit Nemo sont donc tous deux gérés par le projet, et non des copies à maintenir à la main. Les `Desktop Actions` restent aussi présentes dans le `.desktop` pour les lanceurs qui les prennent en charge, mais Nemo utilise son propre fichier `.nemo_action` sur le Bureau.

## Utilisation CLI

```bash
screen-dimmer              # toggle normal ↔ atténué
screen-dimmer toggle       # idem
screen-dimmer ui           # ouvre le curseur
screen-dimmer adjust       # alias de ui
screen-dimmer dim          # applique la valeur atténuée mémorisée
screen-dimmer normal       # revient à xrandr 1.00
screen-dimmer reset        # alias de normal
screen-dimmer off          # alias de normal
screen-dimmer up           # augmente la valeur atténuée de 5 %
screen-dimmer down         # diminue la valeur atténuée de 5 %
screen-dimmer startup-reset  # garde-fou silencieux utilisé au login
screen-dimmer 0.40         # mémorise et applique directement 40 %
```

Les valeurs directes sont bornées entre 10 % et 100 %.

## État mémorisé

`~/.local/state/screen-dimmer/value` contient la valeur atténuée préférée.
`~/.local/state/screen-dimmer/mode` contient `normal` ou `dimmed`.

Passer en mode normal ne remplace jamais la valeur atténuée. Ainsi, avec une préférence à `0.37`, le double-clic alterne simplement entre `1.00` et `0.37`.

Sur une ancienne installation sans fichier `mode`, Screen Dimmer inspecte la valeur `Brightness:` exposée par `xrandr --verbose` pour éviter un premier toggle incohérent.

## Configuration

Les constantes principales sont en tête du script :

```bash
MIN="0.10"
MAX="1.00"
STEP="0.05"
DEFAULT_DIM="0.55"
SYSTEM_DEFAULT="1.00"
```

`DEFAULT_DIM` est uniquement la valeur atténuée proposée sur une installation neuve. `SYSTEM_DEFAULT=1.00` est le multiplicateur logiciel neutre.

## Développement

Le dépôt GitHub est la source de vérité. Après modification :

```bash
bash -n screen-dimmer install.sh tests/smoke.sh
./tests/smoke.sh
git diff --check
desktop-file-validate screen-dimmer.desktop screen-dimmer-autostart.desktop
```

Installer localement uniquement après ces contrôles avec `./install.sh`.

## Licence

MIT — voir [LICENSE](LICENSE).
