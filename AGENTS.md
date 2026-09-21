# AGENTS.md

## Source of truth

- Repository: `Kiwinokoto/screen-dimmer`.
- `main` is the installed/releasable source of truth.
- Develop non-trivial changes on a short-lived `dev/*` branch.
- The installed files are generated from the repo with `./install.sh`; do not edit the installed copy first.

## Runtime

- User-scoped Linux/X11 utility: Bash + `xrandr` + Zenity.
- No sudo, daemon, container or external service is required.
- Installed executable: `~/.local/bin/screen-dimmer`.
- Launcher: `~/.local/share/applications/screen-dimmer.desktop`.
- Login safety entry: `~/.config/autostart/screen-dimmer-reset.desktop`.

## Safety invariants

- UI changes brightness live while the window remains open.
- Closing or terminating the UI restores the neutral `xrandr --brightness 1.00` multiplier without erasing the last chosen slider value.
- Login autostart also restores `1.00`, covering hard shutdowns where an EXIT trap cannot run, and likewise does not overwrite the saved preference.
- `1.00` is the neutral software multiplier; hardware/backlight brightness is not changed.
- Keep direct CLI values clamped to 10–100%.

## Current handoff

- Live slider, persistent Zenity window, close reset and login safety reset are implemented.
- `install.sh` owns the installed executable, launcher and autostart entry.
- No known blocker; real desktop behavior should be manually observed after installation if UI polish changes later.

## Verification

Run before promotion/install:

```bash
bash -n screen-dimmer install.sh tests/smoke.sh
./tests/smoke.sh
git diff --check
```
