#!/bin/sh

appname=`basename "$0" | sed s,\.sh$,,`
dirname=`dirname "$0"`
tmp="${dirname#?}"
if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

export LD_LIBRARY_PATH=$dirname
export QT_QPA_PLATFORM_PLUGIN_PATH=$dirname
sed -i "s,Icon=.*,Icon=${dirname}/wpanda.svg,g" "$dirname/../wpanda.desktop"
"$dirname/$appname" "$@"
