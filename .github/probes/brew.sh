#!/usr/bin/env bash
# Probes for PR #546, Homebrew formula and cask (macOS).
#   O4: does the formula build, pass brew style, brew audit and brew test?
#   B2: the formula's sha256 values against the real archives.
#   B3: is Homebrew's Qt single-architecture, and what architectures does the built app get?
#   B1: the minimum macOS the Homebrew-built binary records.
set -uo pipefail
mkdir -p probe-out
report() { echo "- $*" >> probe-out/results.md; }
echo "## Homebrew [O4 B2 B3 B1]" >> probe-out/results.md

curl -fsSL -o src.tar.gz https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/refs/tags/5.2.2.tar.gz
curl -fsSL -o validator.tar.gz https://github.com/pboettch/json-schema-validator/archive/refs/tags/2.4.0.tar.gz
SRC_SHA=$(shasum -a 256 src.tar.gz | cut -d' ' -f1)
VAL_SHA=$(shasum -a 256 validator.tar.gz | cut -d' ' -f1)
[ "$SRC_SHA" = f099a92d7ac4e7babce79b293076946e78eb53830e90a6be26184f368d2ea398 ] && M=match || M=MISMATCH
report "B2 formula source tarball: actual=$SRC_SHA ($M with pinned f099a92d...)"
[ "$VAL_SHA" = 2ea2a4c3fbadb3fb60f9dabdd3bb552d4211385159289ef3427cfbf7168e5cf2 ] && M=match || M=MISMATCH
report "B2 formula json-schema-validator: actual=$VAL_SHA ($M with pinned 2ea2a4c3...)"

python3 - "$SRC_SHA" "$VAL_SHA" <<'PY'
import pathlib, sys
src_sha, val_sha = sys.argv[1:3]
text = pathlib.Path("Packaging/Homebrew/Formula/wiredpanda.rb").read_text()
text = text.replace("f099a92d7ac4e7babce79b293076946e78eb53830e90a6be26184f368d2ea398", src_sha)
text = text.replace("2ea2a4c3fbadb3fb60f9dabdd3bb552d4211385159289ef3427cfbf7168e5cf2", val_sha)
pathlib.Path("probe-wiredpanda.rb").write_text(text)
PY

brew tap-new --no-git local/probe > tap.log 2>&1
FORMULA="$(brew --repo local/probe)/Formula/wiredpanda.rb"
cp probe-wiredpanda.rb "$FORMULA"
# N9 variant: give the json-schema-validator build and the app build the formula's nlohmann-json prefix.
python3 - "$FORMULA" <<'PY'
import pathlib, sys
p = pathlib.Path(sys.argv[1])
t = p.read_text()
a = '"-DBUILD_SHARED_LIBS=OFF",'
b = '"-DCMAKE_PREFIX_PATH=#{validator_prefix}",'
assert t.count(a) == 1 and t.count(b) == 1
t = t.replace(a, a + '\n                      "-DCMAKE_PREFIX_PATH=#{Formula[\'nlohmann-json\'].opt_prefix}",\n                      "-Dnlohmann_json_DIR=#{Formula[\'nlohmann-json\'].opt_share}/cmake/nlohmann_json",')
t = t.replace(b, '"-DCMAKE_PREFIX_PATH=#{validator_prefix};#{Formula["nlohmann-json"].opt_prefix}",')
p.write_text(t)
PY
if [ $? -eq 0 ]; then report "N9 variant applied: nlohmann-json prefix added to the resource and app configure"; else report "N9 variant NOT applied (patch script failed)"; fi

brew style "$FORMULA" > style.log 2>&1
code=$?
report "O4 brew style formula (with correct sha256 values): exit=$code $(tail -1 style.log)"
brew style Packaging/Homebrew/Casks/wiredpanda.rb > cask-style.log 2>&1
code=$?
report "O4 brew style cask: exit=$code $(tail -1 cask-style.log)"

brew trust local/probe > trust.log 2>&1
brew audit --strict local/probe/wiredpanda > audit.log 2>&1
code=$?
report "O4 brew audit --strict formula: exit=$code $(tail -1 audit.log | head -c 300)"

brew install qtbase > qtbase.log 2>&1
code=$?
report "B3 install Homebrew qtbase: exit=$code"
QTCORE=$(find -L "$(brew --prefix qtbase)" -path '*QtCore.framework/Versions/A/QtCore' -type f 2>/dev/null | head -1)
report "B3 Homebrew QtCore architectures: $(lipo -archs "$QTCORE" 2>&1)"
# N9 diagnosis: why does find_package(nlohmann_json) fail against the Homebrew install?
brew install nlohmann-json > nlohmann.log 2>&1
NJ=$(brew --prefix nlohmann-json)
report "N9 nlohmann-json cmake config files: $(find -L "$NJ" -name 'nlohmann_jsonConfig*.cmake' 2>/dev/null | tr '
' ' ')"
mkdir -p nj-probe
printf 'cmake_minimum_required(VERSION 3.16)
project(p NONE)
find_package(nlohmann_json REQUIRED)
' > nj-probe/CMakeLists.txt
cmake -S nj-probe -B nj-probe/build -DCMAKE_PREFIX_PATH="$NJ" --debug-find-pkg=nlohmann_json > nj-find.log 2>&1
NJ_CODE=$?
report "N9 find_package(nlohmann_json) with the Homebrew prefix: exit=$NJ_CODE $(grep -i 'nlohmann' nj-find.log | grep -E -i 'not found|found|reject|Config' | tr '
' ' ' | head -c 400)"

brew install --build-from-source --verbose local/probe/wiredpanda > formula-install.log 2>&1
code=$?
report "O4 brew install --build-from-source: exit=$code"
if [ "$code" -eq 0 ]; then
  BIN="$(brew --prefix wiredpanda)/wiredpanda.app/Contents/MacOS/wiredpanda"
  report "B3 Homebrew-built binary architectures: $(lipo -archs "$BIN" 2>&1)"
  report "B1 Homebrew-built binary minos: $(vtool -show-build "$BIN" 2>&1 | grep -i minos | head -1 | xargs)"
  brew test local/probe/wiredpanda > test.log 2>&1
  report "O4 brew test: exit=$?"
else
  report "O4 first error lines: $(grep -m3 -i 'error' formula-install.log | tr '\n' ' ' | head -c 700)"
fi

cp style.log audit.log qtbase.log formula-install.log nj-find.log probe-out/ 2>/dev/null
exit 0
