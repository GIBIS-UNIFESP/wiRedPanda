#!/usr/bin/env bash
# Probe for PR #546 Main.cpp: under Flatpak, app.setDesktopFileName(FLATPAK_ID) should make the
# Wayland window's xdg-toplevel app_id equal to the app ID. Runs the installed Flatpak under a headless
# sway compositor and reads the app_id from sway's tree. Best effort: each step reports its own result.
# Run after the flatpak build in flatpak.sh, from the same directory.
set -uo pipefail
mkdir -p probe-out
report() { echo "- $*" >> probe-out/results.md; }
echo "## Wayland app_id under Flatpak [C10]" >> probe-out/results.md

# The log is written by the unprivileged shell, so the redirect is fine here.
# shellcheck disable=SC2024
sudo apt-get install -y sway jq > sway-install.log 2>&1
report "C10 install sway and jq: exit=$?"

export XDG_RUNTIME_DIR="$RUNNER_TEMP/xdg-runtime"
mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR"
export WLR_BACKENDS=headless WLR_RENDERER=pixman WLR_LIBINPUT_NO_DEVICES=1 LIBSEAT_BACKEND=noop
: > empty-sway.conf
sway -c empty-sway.conf > sway.log 2>&1 &
SWAY_PID=$!

SWAYSOCK=""
for _ in $(seq 1 30); do
  SWAYSOCK=$(ls "$XDG_RUNTIME_DIR"/sway-ipc.*.sock 2>/dev/null | head -1)
  [ -n "$SWAYSOCK" ] && break
  sleep 1
done
export SWAYSOCK
WAYLAND_DISPLAY=""
for sock in "$XDG_RUNTIME_DIR"/wayland-*; do
  case "$(basename "$sock")" in
    wayland-[0-9]*) WAYLAND_DISPLAY=$(basename "$sock"); break ;;
  esac
done
export WAYLAND_DISPLAY
report "C10 compositor up: swaysock='${SWAYSOCK:-none}' wayland='${WAYLAND_DISPLAY:-none}'"

if [ -n "$SWAYSOCK" ] && [ -n "${WAYLAND_DISPLAY:-}" ]; then
  dbus-run-session -- flatpak run --env=QT_QPA_PLATFORM=wayland io.github.gibis_unifesp.wiRedPanda > app.log 2>&1 &
  sleep 90
  swaymsg -t get_tree 2> tree.err | jq -c '.. | objects | select(.type? == "con" or .type? == "floating_con") | {name, app_id, shell, pid}' > toplevels.txt 2>> tree.err
  report "C10 toplevels under Flatpak Wayland: $(tr '\n' ' ' < toplevels.txt | head -c 500)"
  report "C10 expected app_id: io.github.gibis_unifesp.wiRedPanda"
  pkill -f wiredpanda 2>/dev/null
else
  report "C10 skipped: no compositor"
fi

kill "$SWAY_PID" 2>/dev/null
cp sway.log app.log toplevels.txt tree.err probe-out/ 2>/dev/null
exit 0
