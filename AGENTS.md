# AGENTS.md

## Source of truth

- Repository: `Kiwinokoto/screen-dimmer`.
- `main` is the installed/releasable source of truth.
- Develop non-trivial changes on a short-lived `dev/*` branch when useful.
- Installed files are generated from the repo with `./install.sh`; do not edit installed copies first.

## Runtime

- User-scoped Linux/X11 utility: Bash + `xrandr` + Zenity.
- No sudo, daemon, container or external service is required.
- Installed executable: `~/.local/bin/screen-dimmer`.
- App launcher: `~/.local/share/applications/screen-dimmer.desktop`.
- Desktop shortcut: `Screen Dimmer.desktop` in the XDG Desktop directory.
- Nemo desktop context action: `~/.local/share/nemo/actions/screen-dimmer-adjust.nemo_action`.
- Login safety entry: `~/.config/autostart/screen-dimmer-reset.desktop`.

## UX invariants

- Default invocation is a toggle: neutral `1.00` ↔ remembered dim value.
- Desktop double-click uses that toggle.
- The Cinnamon/Nemo desktop right-click entry `Régler le mode nuit…` is provided by a dedicated `.nemo_action` restricted to `Screen Dimmer.desktop`.
- Standard Desktop Actions remain in the `.desktop` for launchers that support them.
- The UI changes the dim preference live and leaves it applied when closed.
- Returning to normal must never erase the remembered dim preference.
- Login autostart restores `1.00` and marks the mode normal without erasing the dim preference.
- `1.00` is the neutral software multiplier; hardware/backlight brightness is not changed.
- Keep direct CLI values clamped to 10–100%.

## State

- `value`: remembered dim preference.
- `mode`: `normal` or `dimmed`.
- If `mode` is missing, infer the initial mode from `xrandr --verbose` for migration compatibility.

## Verification

Run before promotion/install:

```bash
bash -n screen-dimmer install.sh tests/smoke.sh
./tests/smoke.sh
git diff --check
desktop-file-validate screen-dimmer.desktop screen-dimmer-autostart.desktop
```

Also verify that the repository contains no secrets before changing visibility. The intended repository visibility is public and the project is MIT-licensed.
