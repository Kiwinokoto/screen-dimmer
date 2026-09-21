#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="${SCREEN_DIMMER_BIN_DIR:-$HOME/.local/bin}"
APP_DIR="${SCREEN_DIMMER_APP_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/applications}"
AUTOSTART_DIR="${SCREEN_DIMMER_AUTOSTART_DIR:-${XDG_CONFIG_HOME:-$HOME/.config}/autostart}"
NEMO_ACTION_DIR="${SCREEN_DIMMER_NEMO_ACTION_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/nemo/actions}"
BIN_PATH="$BIN_DIR/screen-dimmer"

if [[ -n "${SCREEN_DIMMER_DESKTOP_DIR:-}" ]]; then
  DESKTOP_DIR="$SCREEN_DIMMER_DESKTOP_DIR"
elif command -v xdg-user-dir >/dev/null 2>&1; then
  DESKTOP_DIR="$(xdg-user-dir DESKTOP)"
else
  DESKTOP_DIR="$HOME/Desktop"
fi

mkdir -p "$BIN_DIR" "$APP_DIR" "$AUTOSTART_DIR" "$DESKTOP_DIR" "$NEMO_ACTION_DIR"
install -m 0755 "$SCRIPT_DIR/screen-dimmer" "$BIN_PATH"

render_desktop_file() {
  local source="$1"
  local destination="$2"
  local mode="$3"
  local tmp="$destination.tmp"

  awk -v bin="$BIN_PATH" '
    /^Exec=screen-dimmer([[:space:]]|$)/ {
      sub(/^Exec=screen-dimmer/, "Exec=\"" bin "\"")
    }
    { print }
  ' "$source" > "$tmp"

  chmod "$mode" "$tmp"
  mv "$tmp" "$destination"
}

render_desktop_file   "$SCRIPT_DIR/screen-dimmer.desktop"   "$APP_DIR/screen-dimmer.desktop"   0644

render_desktop_file   "$SCRIPT_DIR/screen-dimmer.desktop"   "$DESKTOP_DIR/Screen Dimmer.desktop"   0755

render_desktop_file   "$SCRIPT_DIR/screen-dimmer-autostart.desktop"   "$AUTOSTART_DIR/screen-dimmer-reset.desktop"   0644

render_desktop_file   "$SCRIPT_DIR/screen-dimmer.nemo_action"   "$NEMO_ACTION_DIR/screen-dimmer-adjust.nemo_action"   0644

if command -v gio >/dev/null 2>&1; then
  gio set "$DESKTOP_DIR/Screen Dimmer.desktop" metadata::trusted true >/dev/null 2>&1 || true
fi

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "$APP_DIR" >/dev/null 2>&1 || true
fi

printf 'Installed Screen Dimmer:\n'
printf '  app:       %s\n' "$BIN_PATH"
printf '  launcher:  %s\n' "$APP_DIR/screen-dimmer.desktop"
printf '  desktop:   %s\n' "$DESKTOP_DIR/Screen Dimmer.desktop"
printf '  autostart: %s\n' "$AUTOSTART_DIR/screen-dimmer-reset.desktop"
printf '  nemo action: %s\n' "$NEMO_ACTION_DIR/screen-dimmer-adjust.nemo_action"
