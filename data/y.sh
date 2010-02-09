#!/bin/sh
d=`dirname $0`

LD_LIBRARY_PATH=$d/lib  $d/yberbrowser -graphicssystem raster $@ 2>/dev/null
