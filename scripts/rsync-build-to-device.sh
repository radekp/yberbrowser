#!/bin/bash

d=`dirname $0`
. $d/common.sh

if is_sbox; then
    echo "$0: error: run on host"
    exit 1
fi

mode="Relese"
if is_release || is_webkit_release; then
    mode="Debug"
fi

rsync -av \
    $bd/builddir-armv7/qt-lib/ user@device:yber/lib/ \
    $bd/builddir-armv7/WebKitBuild/Release/lib/libQtWebKit.so.4 user@device:yber/lib/libQtWebKit.so.4 \
    $bd/builddir-armv7/yberbrowser user@device:yber \
    $bd/data/y{,d}.sh user@device:yber