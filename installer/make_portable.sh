#!/bin/sh
echo "Extracting deb package"
dpkg-deb -x *.deb /tmp/extract/
echo "Copying Files"
DATADIR=wpanda_portable_linux
mkdir -p $DATADIR
cp /tmp/extract/usr/local/bin/wpanda $DATADIR
cp shared_qt.sh ${DATADIR}/wpanda.sh
cp /tmp/extract/usr/local/lib/wpanda/* $DATADIR
echo "Generating ${DATADIR}.tar.gz"
tar -zcvf $DATADIR.tar.gz $DATADIR

echo "Removing temporary files"
rm -rf $DATADIR
rm -rf /tmp/extract/
