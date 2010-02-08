#!/bin/bash

# installs builddeps for custom qt
# can be run on host or on sb

d=`dirname $0`
. $d/common.sh

if is_sbox; then
    deps=`dpkg-checkbuilddeps ${QT_SRC_DIR}/debian/control 2>&1 |grep "Unmet build dependencies"  | sed -e 's/dpkg-checkbuilddeps: Unmet build dependencies: //'`
else
    deps="sharutils libdbus-1-dev libfreetype6-dev libglib2.0-dev
 libice-dev libjpeg62-dev libpng12-dev libreadline5-dev libsm-dev \
 libtiff4-dev libx11-dev libxcursor-dev libxext-dev libxft-dev \
 libxi-dev libxmu-dev libxrandr-dev libxrender-dev libxt-dev \
 x11proto-core-dev zlib1g-dev libgstreamer-plugins-base0.10-dev \
 libgstreamer0.10-dev libxau-dev libxcb1-dev \
 libxdmcp-dev libexpat1-dev libsqlite3-dev libsqlite3-0 libssl-dev \
 libgtk2.0-dev\
 libgl1-mesa-dev libgl1-mesa-glx
"

fi

if [ -n "$deps" ]; then
    echo "installing deps: $deps"
    $sudo apt-get install $deps
fi