#!/bin/bash

# example on how to get the webkit that works with yberbrowser.

git clone git://gitorious.org/~akoivisto/webkit/anttis-webkit.git webkit && \
cd webkit && git checkout origin/engine-thread -b engine-thread
