#!/bin/bash


d=`dirname $0`
. $d/common.sh

if is_sbox; then
    (cd $QT_SRC_DIR && ./debian/rules clean)
else
    (cd $QT_SRC_DIR && make distclean)
fi