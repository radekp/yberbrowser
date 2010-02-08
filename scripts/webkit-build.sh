#!/bin/bash

d=`dirname $0`
. $d/common.sh

mode="--debug"
if is_release || is_webkit_release; then
    mode="--release"
fi

mkdir -p $builddir

export PATH=${CUSTOM_QT_PREFIX}/bin:$PATH
export WEBKITOUTPUTDIR=$builddir/WebKitBuild
${WEBKIT_SRC_DIR}/WebKitTools/Scripts/build-webkit --qt $mode --makeargs="${cc_vars} ${makeargs}" ${build_webkit_args}