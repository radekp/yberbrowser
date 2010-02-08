#!/bin/bash

d=`dirname $0`
. $d/common.sh

$sudo apt-get install flex \
libxslt1-dev \
bison \
libx11-dev \
libxext-dev \
zlib1g-dev \
libexpat1-dev