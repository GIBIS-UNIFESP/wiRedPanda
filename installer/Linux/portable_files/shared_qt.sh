#!/bin/sh

appname=`basename $0 | sed s,\.sh$,,`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
LD_LIBRARY_PATH=$dirname
export LD_LIBRARY_PATH
sed -i "s,Icon=.*,Icon=${dirname}/wpanda.svg,g" ${dirname}/../wpanda.desktop
$dirname/$appname "$@"
