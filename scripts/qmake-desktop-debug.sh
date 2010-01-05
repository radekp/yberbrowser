#!/bin/bash

qmake ../yberbrowser.pro WEBKIT=local WEBKIT_PATH=/home/kimkinnu/work/webkit-other/ CONFIG+=release CONFIG-=debug DEFINES+=ENABLE_PAINT_DEBUG=1
