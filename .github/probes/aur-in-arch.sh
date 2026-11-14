#!/usr/bin/env bash
# Runs inside archlinux:latest with the repo mounted at /work.
# Renders Packaging/AUR/PKGBUILD the way .github/workflows/aur-publish.yml does (same sed
# expressions), then runs makepkg: build, check() and package().
# The source is the PR's own tree (probe-out/pr-source.tar.gz, made by aur.sh), not the 5.2.2
# tag: the tag predates the PR's install rules, so it would not test them.
#   N5: does check() leave temporary directories behind?
#   N6: does the default configure fetch JSON dependencies from GitHub?
#   C6, C22: which files are installed, and is LICENSE in the package?
set -uo pipefail
cd /work || exit 1
mkdir -p probe-out
report() { echo "- $*" >> probe-out/results.md; }
echo "## AUR path in archlinux [N5 N6 C6 C22]" >> probe-out/results.md

pacman -Sy --noconfirm archlinux-keyring > /dev/null 2>&1
pacman -Su --noconfirm --needed base-devel git cmake ninja qt6-base qt6-multimedia qt6-svg qt6-tools nlohmann-json namcap > probe-out/pacman.log 2>&1
code=$?
report "AUR makedepends install: exit=$code"
pacman -Si json-schema-validator > /dev/null 2>&1
code=$?
report "N6 json-schema-validator in the official repos: $([ "$code" -eq 0 ] && echo yes || echo 'no')"

useradd -m builder
VER=5.2.2
W=/home/builder/wiredpanda
mkdir -p "$W"
cp probe-out/pr-source.tar.gz "$W/wiredpanda-$VER.tar.gz"
SHA=$(sha256sum "$W/wiredpanda-$VER.tar.gz" | cut -d' ' -f1)
sed \
  -e "s/^pkgver=0\$/pkgver=$VER/" \
  -e "s/^sha256sums=('SKIP')\$/sha256sums=('$SHA')/" \
  -e "s|^source=.*\$|source=(\"wiredpanda-$VER.tar.gz\")|" \
  Packaging/AUR/PKGBUILD > "$W/PKGBUILD"
report "render: pkgver line count $(grep -c "^pkgver=$VER\$" "$W/PKGBUILD"), sha256 line count $(grep -c "^sha256sums=('$SHA')\$" "$W/PKGBUILD"), source is the PR archive"

namcap "$W/PKGBUILD" > probe-out/namcap.txt 2>&1
code=$?
report "namcap on the rendered PKGBUILD: exit=$code, $(wc -l < probe-out/namcap.txt) lines of output"

chown -R builder:builder "$W"
su builder -c "cd $W && makepkg -f --noconfirm --nodeps" > probe-out/makepkg.log 2>&1
code=$?
report "makepkg (build, check, package): exit=$code"
report "check() ctest summary: $(grep -E 'tests passed|tests failed' probe-out/makepkg.log | tail -1)"
report "N6 configure fetched dependencies: $(grep -c -E 'json-src|json-schema-validator-src|FetchContent' probe-out/makepkg.log) matching log lines"
report "N5 temporary directories left in /tmp: $(ls -d /tmp/tmp.* 2>/dev/null | wc -l)"
report "C6 installed files: $(grep -c '^-- Installing:' probe-out/makepkg.log) entries; metainfo: $(grep -c 'share/metainfo/' probe-out/makepkg.log); 256px icon: $(grep -c '256x256/apps/wpanda.png' probe-out/makepkg.log); svg: $(grep -c 'scalable/apps/wpanda.svg' probe-out/makepkg.log)"

PKG=$(ls "$W"/wiredpanda-$VER-*.pkg.tar.zst 2>/dev/null | head -1)
if [ -n "$PKG" ]; then
  report "C22 LICENSE in package: $(bsdtar -tf "$PKG" | grep -c 'share/licenses/wiredpanda/LICENSE')"
  report "package contents: $(bsdtar -tf "$PKG" | grep -E 'usr/bin/wiredpanda|share/applications/|share/metainfo/|share/icons/hicolor/256x256|scalable/apps' | tr '\n' ' ')"
else
  report "package: no .pkg.tar.zst produced"
fi
exit 0
