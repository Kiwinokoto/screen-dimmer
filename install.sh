#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="${SCREEN_DIMMER_BIN_DIR:-$HOME/.local/bin}"
APP_DIR="${SCREEN_DIMMER_APP_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/applications}"
AUTOSTART_DIR="${SCREEN_DIMMER_AUTOSTART_DIR:-${XDG_CONFIG_HOME:-$HOME/.config}/autostart}"
BIN_PATH="$BIN_DIR/screen-dimmer"

mkdir -p "$BIN_DIR" "$APP_DIR" "$AUTOSTART_DIR"
install -m 0755 "$SCRIPT_DIR/screen-dimmer" "$BIN_PATH"

install_desktop_file() {
  local source="$1"
  local destination="$2"
  local command="$3"
  local tmp="$destination.tmp"

  awk -v exec_line="Exec=$command" '
    /^Exec=/ {
      print exec_line
      found = 1
      next
    }
    { print }
    END {
      if (!found) print exec_line
    }
  ' "$source" > "$tmp"
  chmod 0644 "$tmp"
  mv "$tmp" "$destination"
}

install_desktop_file \
  "$SCRIPT_DIR/screen-dimmer.desktop" \
  "$APP_DIR/screen-dimmer.desktop" \
  "\"$BIN_PATH\" ui"

install_desktop_file \
  "$SCRIPT_DIR/screen-dimmer-autostart.desktop" \
  "$AUTOSTART_DIR/screen-dimmer-reset.desktop" \
  "\"$BIN_PATH\" startup-reset"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "$APP_DIR" >/dev/null 2>&1 || true
fi

printf 'Installed Screen Dimmer:\n'
printf '  app:       %s\n' "$BIN_PATH"
printf '  launcher:  %s\n' "$APP_DIR/screen-dimmer.desktop"
printf '  autostart: %s\n' "$AUTOSTART_DIR/screen-dimmer-reset.desktop"
