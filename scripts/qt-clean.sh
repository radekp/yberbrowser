#!/bin/bash


d=`dirname $0`
. $d/common.sh

if is_sbox; then
    (cd $QT_SRC_DIR && $sudo ./debian/rules clean)
else
    (cd $QT_SRC_DIR && $sudo make distclean)
fi
