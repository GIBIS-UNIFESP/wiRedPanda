#!/bin/bash
VERSION=$(cat ~/projetos/wiRedPanda/includes.pri | grep "VERSION = " | awk '{split($0,a,"="); gsub (" ", "", a[2]); print a[2] }')
BUILD_TREE="wpanda-${VERSION}"
rm -rf $BUILD_TREE

echo "Generating build tree for ${BUILD_TREE}.deb"

mkdir -p ${BUILD_TREE}/DEBIAN
mkdir -p ${BUILD_TREE}/usr/local/bin
mkdir -p ${BUILD_TREE}/usr/local/lib/wpanda
mkdir -p ${BUILD_TREE}/usr/local/share/applications
mkdir -p ${BUILD_TREE}/usr/local/share/xml/misc
mkdir -p ${BUILD_TREE}/usr/local/share/doc

for SIZE in 26x26 32x32 48x48 64x64 128x128; do
  ICONDIR_SZ="${BUILD_TREE}/usr/local/share/icons/hicolor/${SIZE}/apps/"
  mkdir -p $ICONDIR_SZ
  cp "${PWD}/../../app/resources/icons/${SIZE}/*" "$ICONDIR_SZ"
done;

tree ${BUILD_TREE}

echo \
"Package: wpanda
Version: ${VERSION}
Installed-Size: 53000
Maintainer: Wired Panda Team <grupo-software-de-circuitos-digitais@googlegroups.com>
Section: utils
Source: https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/v${VERSION}.tar.gz
Homepage: gibis-unifesp.github.io/wiRedPanda
Architecture: amd64
Priority: standard
Description: Learn about logic circuits and simulate them in an easy and friendly way.
 wiRed Panda is a free software designed to help students to learn about logic circuits.
 It was developed by the students of the Federal University of São Paulo.
" > "${BUILD_TREE}/DEBIAN/config"

echo \
"#!/bin/sh

set -e

case "$1" in
configure)
desktop-file-install /usr/local/share/applications/wpanda.desktop
xdg-mime install --mode system /usr/local/share/xml/misc/wpanda-mime.xml
xdg-mime default /usr/local/share/applications/wpanda.desktop application/x-wpanda
xdg-icon-resource install --context mimetypes --size 128  /usr/local/share/icons/hicolor/128x128/apps/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 64  /usr/local/share/icons/hicolor/64x64/apps/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 48  /usr/local/share/icons/hicolor/48x48/apps/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 32  /usr/local/share/icons/hicolor/32x32/apps/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 26  /usr/local/share/icons/hicolor/26x26/apps/wpanda-file.png application-x-wpanda
;;
esac" > "${BUILD_TREE}/DEBIAN/postinst"
