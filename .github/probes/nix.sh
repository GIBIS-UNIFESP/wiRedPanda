#!/usr/bin/env bash
# Probes for PR #546 Nix expression: O3 (build with checkPhase), C21 (Darwin build in the
# sandbox, on macOS runners), N4 (the joao-zip maintainer attribute).
set -uo pipefail
mkdir -p probe-out
report() { echo "- $*" >> probe-out/results.md; }
SYS=$(nix-instantiate --eval --expr builtins.currentSystem | tr -d '"')
echo "## Nix on $SYS [O3 C21 N4]" >> probe-out/results.md

OUT=$(nix-build --no-out-link .github/probes/nix-probe.nix 2> nix-build.err)
code=$?
report "O3 nix-build (build and checkPhase) on $SYS: exit=$code"
report "O3 checkPhase result: $(grep -i 'tests passed\|tests failed' nix-build.err | tail -1)"
if [ "$code" -eq 0 ]; then
  report "O3 outputs: $(ls "$OUT"/bin "$OUT"/Applications 2>/dev/null | tr '\n' ' ')"
else
  report "O3 first error: $(grep -m2 -i 'error' nix-build.err | tr '\n' ' ' | head -c 500)"
fi

if [ "$SYS" = x86_64-linux ]; then
  nix-build --no-out-link .github/probes/nix-fonts.nix > nix-fonts.log 2> nix-fonts.err
  code=$?
  report "N10 variant with DejaVu fonts and FONTCONFIG_FILE: nix-build exit=$code $(grep -i 'tests passed\|tests failed' nix-fonts.err | tail -1)"
fi
nix-instantiate --eval --strict .github/probes/nix-n4.nix > n4.out 2> n4.err
code=$?
report "N4 meta.maintainers evaluation on $SYS: exit=$code $(grep -m1 -i 'error' n4.err | head -c 300)"

cp nix-build.err nix-fonts.err n4.err probe-out/ 2>/dev/null
exit 0
