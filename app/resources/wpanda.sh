 #!/bin/sh
 appname=`basename $0 | sed s,\.sh$,,`

 dirname=`dirname $0`
 tmp="${dirname#?}"

 if [ "${dirname%$tmp}" != "/" ]; then
 dirname=$PWD/$dirname
 fi
 export LD_LIBRARY_PATH=/usr/lib/wpanda:${LD_LIBRARY_PATH}
 export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/wpanda
 $dirname/$appname "$@"
