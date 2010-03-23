#!/bin/bash

d=`dirname $0`
. $d/common.sh

$sudo apt-get install \
flex \
libxslt1-dev \
bison \
libx11-dev \
libgconf2-dev \
libxext-dev \
zlib1g-dev \
libexpat1-dev \
libsqlite3-dev \
libxrender-dev \
libfontconfig1-dev \
libqt4-dev

