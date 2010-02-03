#!/bin/sh

if [ ! -e "`which convert`" ]; then
    echo "run on host and apt-get install imagemagick"
    exit 1
fi

res="24x24 40x40 64x64"
for r in $res; do
    mkdir -p data/icon/$r/hildon/
    convert data/yberbrowser.png -filter cubic -resize $r data/icon/$r/hildon/yberbrowser.png
done