#!/bin/bash

builddir=${builddir:-builddir}

webkit_builddir=${webkit_builddir:-webkit/WebKitBuild/Release}

qtlibdir=${qtlibdir:-/opt/qt4-maemo5/lib}

arch=${arch:-$SBOX_DPKG_INST_ARCH}

version=${version:-0.0.1-1}


pkgname=yberbrowser_${version}_${arch}

if [ -e pkg ]; then
    echo "pkg dir exists. delete it"
    exit 1
fi

if [ ! -e $builddir ]; then
    echo "specify builddir"
    exit 1
fi

if [ ! -e $webkit_builddir ]; then
    echo "specify webkit_builddir"
    exit 1
fi

if [ ! -e $qtlibdir ]; then
    echo "specify qtlibdir"
    exit 1
fi

if [ ! -e "`which chrpath`" ]; then
    echo "run: fakeroot apt-get install chrpath"
    exit 1
fi


mkdir -p pkg/$pkgname/DEBIAN

cat > pkg/$pkgname/DEBIAN/control <<EOF
Package: yberbrowser
Version: $version
Section: Web
Priority: optional
Architecture: $arch
Depends: libc6 (>= 2.5.0-1), libdbus-1-3 (>= 1.1.4), libdbus-glib-1-2 (>= 0.76), libgcc1 (>= 1:4.2.1), libgconf2-6 (>= 2.13.5), libglib2.0-0 (>= 2.20.0), libstdc++6 (>= 4.2.1), libx11-6
Installed-Size: 164
Maintainer: Kimmo Kinnunen <kimmo.t.kinnunen@nokia.com>
Description: Simple browser
    Simple browser (WebKit)
Maemo-Icon-26: 
 iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAABmJLR0QA/wD/AP+g
 vaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH1gURDQoYya0JlwAAAU9J
 REFUSMftlL1KA0EUhb/NZl/ggnHQxsJUxt5CUucVJCCkDfgyKdIGG5/A0s5HEBtJ
 EdDAQGBgmw0YJmMzgXXYza5CtNkDW9zZw5z7c+ZCgwb/Ai3i9sVl/Bq8RIs4LRK1
 gJDsKvJyNXmJMuYTsMoY1zpgozaABdYArQNPZQ1kfyGU7SpqVwxzAMwABWhgpIwp
 4vWBB+AUWAI3ypjnfEXtPU4bLKx9vErTeCeiRSYF+fTn1j5dp2myE9EiU+DSi3wX
 ymeqRQAmZ3EcA5E/fgO6BULT8zhOcrwXoJdrXRa2Lgps2y2odAUcBUIXQdz78YyC
 SldAp8b7+bXrIv91qjZBietqCc2DjbAt4b2WxJkyZljVujlwp0U0cPxuLcAIuC+4
 dKxFlsDJarvdAGP/b6hFnDImYs+uG3hbO2AB3Jbsur63tQM+fFx3bzZocEB8AdV2
 gJBZgKTwAAAAAElFTkSuQmCC
EOF

cat > pkg/$pkgname/DEBIAN/postinst <<EOF
#!/bin/sh
set -e
gtk-update-icon-cache -f /usr/share/icons/hicolor
chown -R user:users /home/user/.yberbrowser
exit 0
EOF
chmod +x pkg/$pkgname/DEBIAN/postinst

mkdir -p pkg/$pkgname/usr/share/applications/hildon
cp data/yberbrowser*.desktop pkg/$pkgname/usr/share/applications/hildon

mkdir -p pkg/$pkgname/usr/share/icons/hicolor/
cp -r data/icon/* pkg/$pkgname/usr/share/icons/hicolor/

mkdir -p pkg/$pkgname/opt/yberbrowser/lib
cp -L $qtlibdir/lib*.so.4 pkg/$pkgname/opt/yberbrowser/lib
#cp -L $qtlibdir/libphonon.so.4.3 pkg/$pkgname/opt/yberbrowser/lib
if [ -e $qtlibdir/qt4/plugins ]; then
    cp -r $qtlibdir/qt4/plugins pkg/$pkgname/opt/yberbrowser/
else
    cp -r $qtlibdir/../plugins pkg/$pkgname/opt/yberbrowser/
fi
cat > pkg/$pkgname/opt/yberbrowser/qt.conf<<EOF
[Paths]
Plugins = /opt/yberbrowser/plugins
EOF
cp pkg/$pkgname/opt/yberbrowser/qt.conf pkg/$pkgname/opt/yberbrowser/lib

mkdir -p pkg/$pkgname/opt/yberbrowser/

cp $builddir/yberbrowser pkg/$pkgname/opt/yberbrowser/
chrpath -d pkg/$pkgname/opt/yberbrowser/yberbrowser

cp -L  $webkit_builddir/lib/libQtWebKit.so.4 pkg/$pkgname/opt/yberbrowser/lib
strip pkg/$pkgname/opt/yberbrowser/lib/*
cp -L  $webkit_builddir/bin/QtWebProcess pkg/$pkgname/opt/yberbrowser/lib

cp data/y.sh pkg/$pkgname/opt/yberbrowser/

mkdir -p pkg/$pkgname/home/user/.yberbrowser
cp -r data/_yberbrowser/* pkg/$pkgname/home/user/.yberbrowser

cd pkg
dpkg-deb --build $pkgname

