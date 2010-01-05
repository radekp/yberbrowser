#!/bin/bash

qmake ../yberbrowser.pro WEBKIT=local WEBKIT_PATH=/home/kimkinnu/work/webkit-other/ CONFIG+=release CONFIG-=debug $@
