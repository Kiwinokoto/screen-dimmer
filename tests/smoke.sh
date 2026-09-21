#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/screen-dimmer-test.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

FAKE_BIN="$TMP/bin"
STATE_HOME="$TMP/state"
LOG="$TMP/xrandr.log"
CURRENT="$TMP/current"
mkdir -p "$FAKE_BIN" "$STATE_HOME"
printf '1.00\n' > "$CURRENT"

cat > "$FAKE_BIN/xrandr" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

if [[ "${1:-}" == "--query" ]]; then
  echo "eDP-1 connected primary 1920x1080+0+0"
  exit 0
fi

if [[ "${1:-}" == "--verbose" ]]; then
  echo "eDP-1 connected primary 1920x1080+0+0"
  printf '    Brightness: %s\n' "$(cat "$SCREEN_DIMMER_TEST_CURRENT")"
  exit 0
fi

if [[ "${1:-}" == "--output" && "${3:-}" == "--brightness" ]]; then
  printf '%s\n' "${4:-}" >> "$SCREEN_DIMMER_TEST_LOG"
  printf '%s\n' "${4:-}" > "$SCREEN_DIMMER_TEST_CURRENT"
  exit 0
fi

exit 2
EOF

cat > "$FAKE_BIN/zenity" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
printf '%s\n' 35 40
EOF

chmod +x "$FAKE_BIN/xrandr" "$FAKE_BIN/zenity"

run_dimmer() {
  env     PATH="$FAKE_BIN:$PATH"     XDG_STATE_HOME="$STATE_HOME"     SCREEN_DIMMER_TEST_LOG="$LOG"     SCREEN_DIMMER_TEST_CURRENT="$CURRENT"     "$ROOT/screen-dimmer" "$@"
}

: > "$LOG"
run_dimmer 0.05 >/dev/null
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.10" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/mode")" == "dimmed" ]]
[[ "$(cat "$CURRENT")" == "0.10" ]]

run_dimmer normal >/dev/null
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.10" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/mode")" == "normal" ]]
[[ "$(cat "$CURRENT")" == "1.00" ]]

run_dimmer toggle >/dev/null
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.10" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/mode")" == "dimmed" ]]
[[ "$(cat "$CURRENT")" == "0.10" ]]

run_dimmer toggle >/dev/null
[[ "$(cat "$STATE_HOME/screen-dimmer/mode")" == "normal" ]]
[[ "$(cat "$CURRENT")" == "1.00" ]]

printf '0.30\n' > "$STATE_HOME/screen-dimmer/value"
printf 'normal\n' > "$STATE_HOME/screen-dimmer/mode"
: > "$LOG"
run_dimmer ui >/dev/null

expected=$'0.30\n0.35\n0.40'
[[ "$(cat "$LOG")" == "$expected" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.40" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/mode")" == "dimmed" ]]
[[ "$(cat "$CURRENT")" == "0.40" ]]

printf '0.42\n' > "$STATE_HOME/screen-dimmer/value"
printf 'dimmed\n' > "$STATE_HOME/screen-dimmer/mode"
: > "$LOG"
run_dimmer startup-reset >/dev/null
[[ "$(cat "$LOG")" == "1.00" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.42" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/mode")" == "normal" ]]
[[ "$(cat "$CURRENT")" == "1.00" ]]

rm -f "$STATE_HOME/screen-dimmer/mode"
printf '0.37\n' > "$CURRENT"
run_dimmer toggle >/dev/null
[[ "$(cat "$STATE_HOME/screen-dimmer/mode")" == "normal" ]]
[[ "$(cat "$CURRENT")" == "1.00" ]]

INSTALL_BIN="$TMP/install-bin"
INSTALL_APPS="$TMP/install-apps"
INSTALL_AUTOSTART="$TMP/install-autostart"
INSTALL_DESKTOP="$TMP/install-desktop"
INSTALL_NEMO_ACTIONS="$TMP/install-nemo-actions"

SCREEN_DIMMER_BIN_DIR="$INSTALL_BIN" SCREEN_DIMMER_APP_DIR="$INSTALL_APPS" SCREEN_DIMMER_AUTOSTART_DIR="$INSTALL_AUTOSTART" SCREEN_DIMMER_DESKTOP_DIR="$INSTALL_DESKTOP" SCREEN_DIMMER_NEMO_ACTION_DIR="$INSTALL_NEMO_ACTIONS"   "$ROOT/install.sh" >/dev/null

[[ -x "$INSTALL_BIN/screen-dimmer" ]]
[[ -x "$INSTALL_DESKTOP/Screen Dimmer.desktop" ]]
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" toggle"   "$INSTALL_APPS/screen-dimmer.desktop"
grep -Fq "Actions=Adjust;Normal;Dim;"   "$INSTALL_APPS/screen-dimmer.desktop"
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" ui"   "$INSTALL_DESKTOP/Screen Dimmer.desktop"
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" normal"   "$INSTALL_DESKTOP/Screen Dimmer.desktop"
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" dim"   "$INSTALL_DESKTOP/Screen Dimmer.desktop"
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" startup-reset"   "$INSTALL_AUTOSTART/screen-dimmer-reset.desktop"
grep -Fq "Name=Régler le mode nuit…"   "$INSTALL_NEMO_ACTIONS/screen-dimmer-adjust.nemo_action"
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" ui"   "$INSTALL_NEMO_ACTIONS/screen-dimmer-adjust.nemo_action"
grep -Fq "Files=Screen Dimmer.desktop;"   "$INSTALL_NEMO_ACTIONS/screen-dimmer-adjust.nemo_action"
grep -Fq "Conditions=desktop;"   "$INSTALL_NEMO_ACTIONS/screen-dimmer-adjust.nemo_action"

echo "screen-dimmer smoke tests: OK"
