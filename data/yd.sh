#!/bin/sh
d=`dirname $0`

QT_PLUGIN_DIR=$d/lib/qt4/plugins \
LD_LIBRARY_PATH=$d/lib  $d/yberbrowser -graphicssystem raster $@
