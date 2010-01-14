#!/bin/bash

qmake ../yberbrowser.pro CONFIG+=debug CONFIG-=release WEBKIT=local WEBKIT_PATH=`pwd`/../webkit WEBKIT_BUILD_MODE=Release $@
