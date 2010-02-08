#!/bin/sh

d=`dirname $0`
. $d/common.sh

if [ -z "$http_proxy" ]; then
    echo "error: setup http_proxy env var"
fi

extras_fn=/etc/apt/sources.list.d/extras.list
# for bare scratchbox
if [ ! -e "$extras_fn" ]; then
    fakeroot tee "$extras_fn" <<EOF
deb http://repository.maemo.org/extras-devel/ fremantle free non-free
deb-src http://repository.maemo.org/extras-devel/ fremantle free
EOF
fi

fakeroot apt-get update
fakeroot apt-get -y install maemo-sdk-dev
fakeroot apt-get -y install \
osso-af-sb-startup \
hildon-input-method-plugins-western \
hildon-home \
hildon-desktop \
hildon-status-menu \
hildon-control-panel \
theme-config \
hildon-theme-devel \
osso-browser \
libxrender-dev \
libsms-utils-dev \
librtcom-call-ui-dev \
libprofile-dev \