#!/bin/bash

d=`dirname $0`
. $d/common.sh

mode="-debug"
if is_release; then
    mode="-release"
fi

# commented out because it might be the cause for fail on sb-armel
#if [ -n "${cc_vars}" ]; then
#    eval ${cc_vars}
#    export CC
#    export CXX
#fi
#       CC=$CC \
#       CXX=$CXX \


conf_opts="-no-declarative \
 -no-svg \
 -no-webkit \
 -no-qt3support \
 -developer-build \
 -optimized-qmake \
 ${mode} \
 -prefix '${CUSTOM_QT_PREFIX}'
 -plugindir '${CUSTOM_QT_PREFIX}/lib/qt4/plugins' \
 -translationdir '${CUSTOM_QT_PREFIX}/share/qt4/translations' \
"
# commented out because it might be the cause for fail on sb-armel
# -nomake 'tools examples demos docs' 

if is_sbox; then

    (cd $QT_SRC_DIR && \
        DEB_BUILD_OPTIONS=parallel \
        EXTRA_CONFIGURE_OPTS=$conf_opts \
         ./debian/rules build && \
        $sudo make install && \
        cp -r $CUSTOM_QT_PREFIX/lib $builddir/qt-lib/ #for packaging and rsync-to-device, unfortunately
    )

else
#    export CC
#    export CXX
    (cd $QT_SRC_DIR \
        ./configure -opensource -confirm-license $conf_opts && \
        make $makeargs $cc_vars &&\
        $sudo make install)
fi


