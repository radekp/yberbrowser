!isEqual(QT_MAJOR_VERSION, 4) | !greaterThan(QT_MINOR_VERSION, 5): {
    error("Use Qt4.6");
}

TEMPLATE = app
CONFIG -= app_bundle

isEmpty(PREFIX) {
    PREFIX = /usr
}
DATADIR = $$PREFIX/share
BINDIR = $$PREFIX/bin


QT += network opengl
macx:QT+=xml

disable_tile_cache {

} else {
    DEFINES+=WEBKIT_SUPPORTS_TILE_CACHE=1
}

contains(WEBKIT, system) {
    QT += webkit
} else: contains(WEBKIT, custom): {
    #use custom webkit pkg from
    # http://gitorious.org/qtwebkit-custom-pkg/qtwebkit-custom-pkg
    CONFIG += link_pkgconfig
    PKGCONFIG += QtWebKit-custom
} else: contains(WEBKIT, local): {
    isEmpty(WEBKIT_PATH) {
        error(Use WEBKIT_PATH=/path to specify where WebKit checkout is)
    }
    !exists($$WEBKIT_PATH/WebKit.pro) {
        error(WebKit does not exist in path $$WEBKIT_PATH)
    }
    isEmpty(WEBKIT_BUILD_PATH) {
        debug {
           WEBKIT_BUILD_MODE=Debug
        } else {
           WEBKIT_BUILD_MODE=Release
        }
        WEBKIT_BUILD_PATH=$$WEBKIT_PATH/WebKitBuild/$$WEBKIT_BUILD_MODE
    }

    WEBKIT_LIB_FILE=$$WEBKIT_BUILD_PATH/lib/libQtWebKit.so

    !exists($$WEBKIT_LIB_FILE) {
       error(WebKit has not been built to path '$$WEBKIT_BUILD_PATH' (lib $$WEBKIT_LIB_FILE does not exist). Build it or specify WEBKIT_BUILD_PATH)
    }

    LIBPATH += $$WEBKIT_BUILD_PATH/lib
    LIBS += -lQtWebKit

    INCLUDEPATH += $$WEBKIT_PATH/WebKit/qt/Api \
     $$WEBKIT_PATH/ \
     $$WEBKIT_PATH/JavaScriptCore/ForwardingHeaders

    # need to set rpath, so that app can be run during development from
    # the build dir.  after installing, we need to do run 'chrpath -d' on
    # the executable.
    QMAKE_RPATHDIR = $$WEBKIT_BUILD_PATH/lib $$QMAKE_RPATHDIR
} else {
  error(Specify WEBKIT build mode: WEBKIT=[system|custom|local])
}

symbian {
    TARGET.UID3 = 0xA000E544
    TARGET.CAPABILITY = ReadUserData WriteUserData NetworkServices
}



target.path = $$BINDIR
INSTALLS += target

HEADERS = \
  src/AutoScroller.h

SOURCES = \
  src/main.cpp \
  src/AutoScroller.cpp
