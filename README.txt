This is based on QGVLauncher

Credits: QGVLauncher committers

Supported build envs:
   * host (linux host)
   * sb-arm
   * sb-i486

---+ File structure

By default, we need custom version of qt and webkit.

The scripts 'git clone' corresponding sources to following dirs:
   * ./qt4-maemo5 -- qt in fremantle sb
   * ./qt         -- qt in host
   * ./webkit     -- webkit

---+ Setting up SB environment from scratch, Fremantle

Start with empty computer, SB installed.

In host:

sudo apt-get install maemo-assistant

---+ Create SB targets

In host:

maemo-assistant fremantle -f -r 2.2009.51-1 -t maemo5-w51

---+ Install

In SB:
./sb-install-deps-fremantle.sh
./scripts/qt-install-builddeps.sh
./webkit-install-builddeps.sh

---+ Clone

In SB:
./scripts/qt-clone.sh
./scripts/webkit-clone.sh

---+ Build

In SB:
./scripts/qt-build.sh
./scripts/webkit-build.sh


---+ Building yberbrowser

Build webkit as above.

In SB:

cd builddir-armel
../scripts/qmake-dev.sh
make
./yberbrowser


---+ Build notes

Build scripts will one of following dirs:
   * builddir-host
   * builddir-i486
   * builddir-armel

WebKit is built to builddir, so that source tree remains really
prestine and git is fast.

Builds are always debug. You can run build scripts with '--release' to
compile release versions.


---++ Build with system webkit package


mkdir builddir-host && cd builddir-host
qmake ../yberbrowser.pro WEBKIT=system



Build with "custom" webkit package
----------------------------------

mkdir builddir && cd builddir
qmake ../yberbrowser.pro WEBKIT=custom

Custom webkit package is a packaged webkit for custom builds:
http://gitorious.org/qtwebkit-custom-pkg/qtwebkit-custom-pkg


Build with locally built webkit
-------------------------------

cd webkit
WebKitTools/Scripts/build-webkit --qt
cd ..

cd yberbrowser
mkdir builddir && cd builddir
qmake ../yberbrowser.pro WEBKIT=local WEBKIT_PATH=`pwd`/../webkit/



Variables for qmake
-------------------

WEBKIT=[local|system|custom]

If selecting WEBKIT=local

WEBKIT_PATH= full path to webkit sources, where webkit is also built by default
WEBKIT_BUILD_PATH= override the full path where the webkit has been built to
WEBKIT_BUILD_MODE= override what mode the webkit has been built (used unless WEBKIT_BUILD_PATH has been defined)



Day-to-day development
---------------------

See scripts/ -dir for qmake build scripts, etc

To debug webkit debug version using ubuntu karmic, use following:
ulimit -s 65535


Run examples
------------


Help:
./yberbrowser -h

Fullscreen:
./yberbrowser /path/to/file

Windowed, raster:
./yberbrowser -graphicssystem raster -w

Windowed, raster, GLWidget for QGraphicsView:
./yberbrowser -graphicssystem raster -w -g http://slashdot.org


icons
----
Thanks to ICONS etc.
Author:	Mysitemyway.com

http://icons.mysitemyway.com/terms-of-use/
All royalty free stock icons, clip art and other design resources on this website are free for use in both personal and commercial projects.

