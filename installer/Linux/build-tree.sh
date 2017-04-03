#!/bin/bash
set -e
VERSION=$(cat ~/projetos/wiRedPanda/includes.pri | grep "VERSION = " | awk '{split($0,a,"="); gsub (" ", "", a[2]); print a[2] }')
BUILD_TREE="wpanda-${VERSION}"
PROJECTPATH=$(readlink -e "${PWD}/../../app/")
QT_LIBRARY_PATH="/opt/qt58/gcc_64/lib/"
QT_PLATFORMS_PATH="/opt/qt58/gcc_64/plugins/platforms/"
DEPENDENCIES_PATH="/usr/lib/x86_64-linux-gnu/"
rm -rf $BUILD_TREE

echo "Generating build tree for ${BUILD_TREE}.deb"
echo "Project path is ${PROJECTPATH}"
mkdir -p ${BUILD_TREE}/DEBIAN
mkdir -p ${BUILD_TREE}/usr/local/bin
mkdir -p ${BUILD_TREE}/usr/local/lib/wpanda
mkdir -p ${BUILD_TREE}/usr/local/share/applications
mkdir -p ${BUILD_TREE}/usr/local/share/xml/misc
mkdir -p ${BUILD_TREE}/usr/local/share/doc

for SIZE in 26x26 32x32 48x48 64x64 128x128; do
  ICONDIR_SZ=${BUILD_TREE}/usr/local/share/icons/hicolor/${SIZE}/apps/
  mkdir -p $ICONDIR_SZ
  cp $PROJECTPATH/resources/icons/${SIZE}/* $ICONDIR_SZ
done;

cp $PROJECTPATH/resources/wpanda.desktop  ${BUILD_TREE}/usr/local/share/applications/
cp $PROJECTPATH/resources/wpanda.sh       ${BUILD_TREE}/usr/local/bin/
cp $PROJECTPATH/resources/wpanda.desktop  ${BUILD_TREE}/usr/local/share/applications/
cp $PROJECTPATH/resources/wpanda-mime.xml ${BUILD_TREE}/usr/local/share/xml/misc

cp $QT_LIBRARY_PATH/libQt5PrintSupport.so.5 ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libQt5XcbQpa.so.5       ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libQt5DBus.so.5         ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libQt5Charts.so.5       ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libQt5Widgets.so.5      ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libQt5Gui.so.5          ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libQt5Core.so.5         ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libicui18n.so.56        ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libicuuc.so.56          ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_LIBRARY_PATH/libicudata.so.56        ${BUILD_TREE}/usr/local/lib/wpanda
cp $QT_PLATFORMS_PATH/libqxcb.so            ${BUILD_TREE}/usr/local/lib/wpanda
cp $DEPENDENCIES_PATH/libX11-xcb.so.1       ${BUILD_TREE}/usr/local/lib/wpanda
cp $DEPENDENCIES_PATH/libxcb.so.1           ${BUILD_TREE}/usr/local/lib/wpanda
cp $DEPENDENCIES_PATH/libxcb-dri2.so.0      ${BUILD_TREE}/usr/local/lib/wpanda
cp $DEPENDENCIES_PATH/libxcb-xfixes.so.0    ${BUILD_TREE}/usr/local/lib/wpanda

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

tree ${BUILD_TREE}
