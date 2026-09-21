#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/screen-dimmer-test.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

FAKE_BIN="$TMP/bin"
STATE_HOME="$TMP/state"
LOG="$TMP/xrandr.log"
mkdir -p "$FAKE_BIN" "$STATE_HOME"

cat > "$FAKE_BIN/xrandr" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

if [[ "${1:-}" == "--query" ]]; then
  echo "eDP-1 connected primary 1920x1080+0+0"
  exit 0
fi

if [[ "${1:-}" == "--output" && "${3:-}" == "--brightness" ]]; then
  printf '%s\n' "${4:-}" >> "$SCREEN_DIMMER_TEST_LOG"
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
  env \
    PATH="$FAKE_BIN:$PATH" \
    XDG_STATE_HOME="$STATE_HOME" \
    SCREEN_DIMMER_TEST_LOG="$LOG" \
    SCREEN_DIMMER_TEST_ZENITY_COUNT="$TMP/zenity-count" \
    "$ROOT/screen-dimmer" "$@"
}

: > "$LOG"
run_dimmer 0.05 >/dev/null
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.10" ]]
[[ "$(tail -n 1 "$LOG")" == "0.10" ]]

run_dimmer reset >/dev/null
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "1.00" ]]
[[ "$(tail -n 1 "$LOG")" == "1.00" ]]
printf '0.30\n' > "$STATE_HOME/screen-dimmer/value"
: > "$LOG"
run_dimmer ui >/dev/null

expected=$'0.30\n0.35\n0.40\n1.00'
[[ "$(cat "$LOG")" == "$expected" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.40" ]]

printf '0.42\n' > "$STATE_HOME/screen-dimmer/value"
: > "$LOG"
run_dimmer startup-reset >/dev/null
[[ "$(cat "$LOG")" == "1.00" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.42" ]]

cat > "$FAKE_BIN/zenity" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
count=0
[[ -f "$SCREEN_DIMMER_TEST_ZENITY_COUNT" ]] && count="$(cat "$SCREEN_DIMMER_TEST_ZENITY_COUNT")"
printf '%s\n' "$((count + 1))" > "$SCREEN_DIMMER_TEST_ZENITY_COUNT"
if [[ "$count" -eq 0 ]]; then
  printf '%s\n' 45 Reset
else
  printf '%s\n' 55
fi
EOF
chmod +x "$FAKE_BIN/zenity"

rm -f "$TMP/zenity-count"
printf '0.30\n' > "$STATE_HOME/screen-dimmer/value"
: > "$LOG"
run_dimmer ui >/dev/null
expected=$'0.30\n0.45\n1.00\n1.00\n0.55\n1.00'
[[ "$(cat "$LOG")" == "$expected" ]]
[[ "$(cat "$STATE_HOME/screen-dimmer/value")" == "0.55" ]]

INSTALL_BIN="$TMP/install-bin"
INSTALL_APPS="$TMP/install-apps"
INSTALL_AUTOSTART="$TMP/install-autostart"

SCREEN_DIMMER_BIN_DIR="$INSTALL_BIN" \
SCREEN_DIMMER_APP_DIR="$INSTALL_APPS" \
SCREEN_DIMMER_AUTOSTART_DIR="$INSTALL_AUTOSTART" \
  "$ROOT/install.sh" >/dev/null

[[ -x "$INSTALL_BIN/screen-dimmer" ]]
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" ui" \
  "$INSTALL_APPS/screen-dimmer.desktop"
grep -Fq "Exec=\"$INSTALL_BIN/screen-dimmer\" startup-reset" \
  "$INSTALL_AUTOSTART/screen-dimmer-reset.desktop"

echo "screen-dimmer smoke tests: OK"
