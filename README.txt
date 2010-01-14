This is based on QGVLauncher

Credits: QGVLauncher committers



Build with system webkit package
--------------------------------

mkdir builddir && cd builddir
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



