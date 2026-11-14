#!/usr/bin/env bash
# Probe for PR #546 B1 impact. On this runner's macOS version:
#   1. Verify the released DMG (checksum against the cask), then run its binary.
#   2. Run the 12.0-target bundle from the macos-floor-build job.
# Records the exit status and stderr. A dyld "Symbol not found" error at launch counts as a failure.
# Limit: --version exits before the GUI opens, so this checks that the binary loads and starts,
# not that the window works.
set -uo pipefail
mkdir -p probe-out
report() { echo "- $*" >> probe-out/results.md; }
OS=$(sw_vers -productVersion)
echo "## Launch on macOS $OS [B1]" >> probe-out/results.md

run_once() { # $1 label, $2 binary path
  QT_QPA_PLATFORM=offscreen "$2" --version > "$1.out" 2> "$1.err"
  code=$?
  report "B1 $1 --version on macOS $OS: exit=$code stdout='$(head -1 "$1.out")' stderr='$(head -c 240 "$1.err" | tr '\n' ' ')'"
}

URL=https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/download/5.2.2/wiRedPanda-5.2.2-macOS-Qt6.9.3.dmg
curl -fsSL -o shipped.dmg "$URL"
GOT=$(shasum -a 256 shipped.dmg | cut -d' ' -f1)
[ "$GOT" = e1a92347c0ed23785e34d939f260a446ba1355470d196ab6bfb03d22b46ca83d ] && report "cask DMG sha256 matches the cask on macOS $OS" || report "cask DMG sha256 MISMATCH on macOS $OS"

MNT="$RUNNER_TEMP/shipped"
hdiutil attach -nobrowse -readonly -mountpoint "$MNT" shipped.dmg > attach.log 2>&1
report "C13 DMG attach: exit=$?"
report "C13 volume name: $(diskutil info "$MNT" 2>/dev/null | awk -F': *' '/Volume Name/{print $2}')"
report "C13 DMG top level: $(ls "$MNT" 2>/dev/null | tr '\n' ' ')"
SHIPPED="$MNT/wiredpanda.app/Contents/MacOS/wiredpanda"
report "B1 shipped binary minos: $(vtool -show-build "$SHIPPED" 2>&1 | grep -i minos | head -1 | xargs)"
run_once shipped "$SHIPPED"
hdiutil detach "$MNT" -quiet > /dev/null 2>&1

if [ -f floor/build-12.zip ]; then
  ditto -x -k floor/build-12.zip floor-run
  FLOOR=$(find floor-run -name wiredpanda -path '*Contents/MacOS/*' | head -1)
  run_once floor12 "$FLOOR"
else
  report "B1 floor12: no bundle artifact (the build job did not produce one)"
fi
exit 0
