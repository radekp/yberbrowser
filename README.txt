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





