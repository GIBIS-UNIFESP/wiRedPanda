#!/bin/sh

set -e
set -v
if [[ $(id -u) -ne 0 ]] ; then echo "Please run as root" ; exit 1 ; fi

INSTALL_DIR="/opt/wpanda/"

rm -rf $INSTALL_DIR

install -d $INSTALL_DIR
install -d $INSTALL_DIR/icons/26x26 $INSTALL_DIR/icons/32x32 $INSTALL_DIR/icons/48x48
install -d $INSTALL_DIR/icons/64x64 $INSTALL_DIR/icons/128x128

install *.so*  $INSTALL_DIR
install wpanda*  $INSTALL_DIR
install icons/26x26/*    $INSTALL_DIR/icons/26x26 
install icons/32x32/*    $INSTALL_DIR/icons/32x32 
install icons/48x48/*    $INSTALL_DIR/icons/48x48
install icons/64x64/*    $INSTALL_DIR/icons/64x64 
install icons/128x128/*  $INSTALL_DIR/icons/128x128

echo \
"[Desktop Entry]
GenericName=wiRED Panda
Name=wiRED Panda
Comment=Learn about logic circuits in an easy and friendly way.
Exec=/bin/sh $INSTALL_DIR/wpanda.sh %f
Icon=$INSTALL_DIR/wpanda.svg
Type=Application
Categories=Utility;
Terminal=false
StartupNotify=false
MimeType=application/x-wpanda;" > $INSTALL_DIR/wpanda.desktop

desktop-file-install $INSTALL_DIR/wpanda.desktop 

xdg-mime install --mode system $INSTALL_DIR/wpanda-mime.xml
xdg-mime default /usr/share/applications/wpanda.desktop application/x-wpanda
xdg-icon-resource install --context mimetypes --size 128  $INSTALL_DIR/icons/128x128/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 64   $INSTALL_DIR/icons/64x64/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 48   $INSTALL_DIR/icons/48x48/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 32   $INSTALL_DIR/icons/32x32/wpanda-file.png application-x-wpanda
xdg-icon-resource install --context mimetypes --size 26   $INSTALL_DIR/icons/26x26/wpanda-file.png application-x-wpanda
