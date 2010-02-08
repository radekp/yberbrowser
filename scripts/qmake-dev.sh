#!/bin/bash


d=`dirname $0`
. $d/common.sh



mode="CONFIG+=debug CONFIG-=release"
if is_release; then
    mode="CONFIG+=release CONFIG-=debug"
fi

webkit_mode="WEBKIT_BUILD_MODE=Debug"
if is_webkit_release; then
    webkit_mode="WEBKIT_BUILD_MODE=Release"
fi

${CUSTOM_QT_PREFIX}/bin/qmake ../yberbrowser.pro $mode WEBKIT=local WEBKIT_PATH=`pwd`/../webkit $webkit_mode $@
