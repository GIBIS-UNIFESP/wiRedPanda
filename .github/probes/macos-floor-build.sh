#!/usr/bin/env bash
# Probe for PR #546 B1: build the app with CMAKE_OSX_DEPLOYMENT_TARGET=12.0 on macos-26
# and read the minimum macOS the binary records. The shipped binary records 26.0 (no target set).
# The bundle is handed to the macos-run job, which launches it on older macOS versions.
set -uo pipefail
mkdir -p probe-out
report() { echo "- $*" >> probe-out/results.md; }
echo "## macOS build, deployment target 12.0 [B1]" >> probe-out/results.md

# Qt comes from the install-qt-action step in the workflow (same action and modules as deploy.yml).
QT="${QT_ROOT_DIR:?QT_ROOT_DIR is set by the install-qt-action step}"
report "setup: Qt 6.9.3 at $QT (install-qt-action, same modules as deploy.yml)"

command -v ninja > /dev/null || brew install ninja

cmake -S . -B build-12 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QT" \
  -DBUILD_TESTING=OFF -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 > configure.log 2>&1
code=$?
report "B1 configure with deployment target 12.0: exit=$code"
cmake --build build-12 --target wiredpanda --parallel > build.log 2>&1
code=$?
report "B1 build with deployment target 12.0: exit=$code"
if [ "$code" -ne 0 ]; then
  report "B1 first error lines: $(grep -m3 -i 'error' build.log | tr '\n' ' ' | head -c 700)"
  cp configure.log build.log probe-out/ 2>/dev/null
  exit 0
fi

APP=$(find build-12 -name wiredpanda.app -type d | head -1)
"$QT/bin/macdeployqt" "$APP" > deploy.log 2>&1
report "B1 macdeployqt bundling: exit=$?"
report "B1 bundle minos: $(vtool -show-build "$APP/Contents/MacOS/wiredpanda" 2>&1 | grep -i minos | head -1 | xargs)"
ditto -c -k --keepParent "$APP" probe-out/build-12.zip
report "B1 bundle zipped for the macos-run job"

cp configure.log build.log deploy.log probe-out/ 2>/dev/null
exit 0
