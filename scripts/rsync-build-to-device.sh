#!/bin/bash

d=`dirname $0`
. $d/common.sh

if is_sbox; then
    echo "$0: error: run on host"
    exit 1
fi

mode="Debug"
if is_release || is_webkit_release; then
    mode="Release"
fi

ssh user@device mkdir -p yber/lib

rsync -av \
    $bd/builddir-armel/qt-lib/lib*.so* user@device:yber/lib/

rsync -av \
    $bd/builddir-armel/qt-lib/qt4 user@device:yber/lib/

rsync -av \
    $bd/builddir-armel/WebKitBuild/$mode/lib/libQtWebKit.so* user@device:yber/lib/

rsync -av \
    $bd/builddir-armel/yberbrowser user@device:yber

rsync -av \
    $bd/data/y*.sh user@device:yber

rsync -av \
    $bd/data/home-qt.conf user@device:yber/qt.conf