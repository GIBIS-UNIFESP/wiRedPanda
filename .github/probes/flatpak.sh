#!/usr/bin/env bash
# Probes for PR #546, Flatpak manifest.
#   O1: does org.kde.Platform 6.11 exist on Flathub?
#   O2: does org.kde.Sdk 6.11 provide lrelease and lupdate?
#   B2: what does the manifest do with its pinned sha256 values?
#   Build with the correct sha256 values, then inspect the installed tree.
set -uo pipefail
mkdir -p probe-out
report() { echo "- $*" >> probe-out/results.md; }
MANIFEST=Packaging/Flatpak/io.github.gibis_unifesp.wiRedPanda.yml

echo "## Flatpak [O1 O2 B2 build]" >> probe-out/results.md

flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
report "setup: flathub remote added, exit=$?"

for ref in org.kde.Platform//6.11 org.kde.Sdk//6.11; do
  flatpak remote-info --user flathub "$ref" > /dev/null 2>&1
  code=$?
  report "O1 remote-info $ref: exit=$code"
done
report "O1 KDE runtimes on flathub: $(flatpak remote-ls --user --runtime --columns=application,branch flathub 2>/dev/null | grep -E '^org\.kde\.(Platform|Sdk)[[:space:]]' | awk '{print $1 "/" $2}' | sort -V | tr '\n' ' ')"

flatpak install --user -y --noninteractive flathub org.kde.Sdk//6.11 org.kde.Platform//6.11 > install.log 2>&1
code=$?
report "O2 install org.kde.Sdk//6.11 and org.kde.Platform//6.11: exit=$code"
report "O2 translation tools in SDK: $(flatpak run --command=sh org.kde.Sdk//6.11 -c 'for t in lrelease lupdate; do command -v "$t" || echo "$t MISSING"; done' 2>&1 | tr '\n' ' ')"

flatpak-builder --user --force-clean --disable-rofiles-fuse --install-deps-from=flathub \
  --repo=probe-repo-asis probe-build-asis "$MANIFEST" > asis.log 2>&1
code=$?
report "B2 manifest as pinned: exit=$code, first checksum message: $(grep -m1 -i 'sha256\|checksum' asis.log | head -c 300)"

python3 - "$MANIFEST" > shas.txt <<'PY'
import hashlib, pathlib, re, sys, urllib.request
path = sys.argv[1]
text = pathlib.Path(path).read_text()
for url, pinned in re.findall(r"url: (\S+)\n\s+sha256: (\w+)", text):
    actual = hashlib.sha256(urllib.request.urlopen(url).read()).hexdigest()
    print(f"{url.rsplit('/', 1)[-1]} pinned={pinned[:12]} actual={actual[:12]} match={pinned == actual}")
    text = text.replace(pinned, actual)
pathlib.Path("probe-flatpak.yml").write_text(text)
PY
report "B2 sources, pinned vs actual: $(tr '\n' ';' < shas.txt)"

flatpak-builder --user --force-clean --disable-rofiles-fuse --install-deps-from=flathub --install \
  --repo=probe-repo probe-build probe-flatpak.yml > build.log 2>&1
code=$?
report "P1 build with correct sha256 values: exit=$code"

if [ "$code" -eq 0 ]; then
  flatpak-builder --run probe-build probe-flatpak.yml sh -c 'export QT_QPA_PLATFORM=offscreen; echo "FLATPAK_ID=$FLATPAK_ID"; wiredpanda --version; echo "desktop:"; ls /app/share/applications; echo "metainfo:"; ls /app/share/metainfo; echo "mime:"; ls /app/share/mime/packages; echo "wpanda.png count: $(find /app/share/icons/hicolor -name wpanda.png | wc -l)"; qmake6 -query QT_VERSION' > run.out 2>&1
  report "P1 inside the sandbox: $(tr '\n' ' ' < run.out | head -c 900)"
fi

cp asis.log install.log shas.txt build.log probe-out/ 2>/dev/null
[ -f run.out ] && cp run.out probe-out/
exit 0
