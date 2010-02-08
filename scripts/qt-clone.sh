#!/bin/bash


# example on how to get qt that works with yberbrowser when built with engine-thread

d=`dirname $0`
. $d/common.sh

if is_sbox; then
    git clone git://gitorious.org/~kimmok/qt/qt-x11-maemo-threadsafer.git qt4-maemo5 && \
    cd qt4-maemo5 && git checkout origin/4.6-fremantle -b 4.6-fremantle
else
    #not sure this works
    git clone git://gitorious.org/~kimmok/qt/qt-x11-maemo-threadsafer.git qt4 && \
    cd qt && git checkout origin/4.6-fremantle -b 4.6-fremantle
fi
