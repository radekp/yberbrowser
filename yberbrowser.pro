WEBKIT=system

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

!symbian {
	MOC_DIR     = $$OUT_PWD/.moc
	OBJECTS_DIR = $$OUT_PWD/.obj
	RCC_DIR     = $$OUT_PWD/.rcc
}

meegotouch {
    DEFINES+=USE_MEEGOTOUCH=1
} else {
    DEFINES+=USE_MEEGOTOUCH=0
}

QT += network
QT += xml xmlpatterns script

contains(QT_CONFIG, opengl): QT += opengl

# Add $$PWD to include path so we can include from 3rdparty/file.h.
# we want to specify '3rdparty/' explicitly to avoid name clashes
# with system include path.

INCLUDEPATH += $$PWD/

contains(QT_CONFIG, maemo5)  {
    QT += maemo5 dbus
}

enable_webkit2 {
    DEFINES+=USE_WEBKIT2=1
} else {
    DEFINES+=USE_WEBKIT2=0
}

!symbian {

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
    !exists($$WEBKIT_PATH/WebKit.pri) {
        error(WebKit does not exist in path $$WEBKIT_PATH)
    }

    # ideally we would like to include WebKit.pri this is not
    # feasible, however, because it will cause build flags to be
    # different. With WebKit.pri we cannot build yberbrowser with
    # debug and webkit in release. This is in turn needed due to
    # gdb segfaulting on debug webkit

    isEmpty(WEBKIT_BUILD_PATH) {
       isEmpty(WEBKIT_BUILD_MODE) {
           debug {
               WEBKIT_BUILD_MODE=Debug
           } else {
               WEBKIT_BUILD_MODE=Release
           }
       }
       WEBKIT_BUILD_PATH=$$OUT_PWD/WebKitBuild/$$WEBKIT_BUILD_MODE
    }

    mac:contains(QT_CONFIG, qt_framework):!CONFIG(webkit_no_framework) {
        # copied unfortunately from WebKit.pri
        WEBKIT_LIB_FILE=$$WEBKIT_BUILD_PATH/lib/QtWebKit.framework/QtWebKit
        LIBS += -framework QtWebKit
        QMAKE_FRAMEWORKPATH = $$WEBKIT_BUILD_PATH/lib $$QMAKE_FRAMEWORKPATH
    } else {
        WEBKIT_LIB_FILE=$$WEBKIT_BUILD_PATH/lib/libQtWebKit.so
        LIBPATH += $$WEBKIT_BUILD_PATH/lib
        LIBS += -lQtWebKit
    }

    !exists($$WEBKIT_LIB_FILE) {
       error(WebKit has not been built to path '$$WEBKIT_BUILD_PATH' (lib $$WEBKIT_LIB_FILE does not exist). Build it or specify WEBKIT_BUILD_PATH)
    }

    INCLUDEPATH += $$WEBKIT_PATH/WebKit/qt/Api \
     $$WEBKIT_PATH/ \
     $$WEBKIT_PATH/JavaScriptCore/ForwardingHeaders \
     $$WEBKIT_BUILD_PATH/include

    # need to set rpath, so that app can be run during development from
    # the build dir.  after installing, we need to do run 'chrpath -d' on
    # the executable.
    QMAKE_RPATHDIR = $$WEBKIT_BUILD_PATH/lib $$QMAKE_RPATHDIR
} else {
  error(Specify WEBKIT build mode: WEBKIT=[system|custom|local])
}
}


symbian {
	TARGET.UID3 = 0xA000E544
	TARGET.CAPABILITY = All -Tcb
	TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
	QT += webkit

    preloadedItems.sources = ./data/_yberbrowser/*.*
    preloadedItems.path = /private/A000E544

    DEPLOYMENT += preloadedItems 
    
    BLD_INF_RULES.prj_exports += \
        "./data/_yberbrowser/1433288371.png      /epoc32/winscw/c/private/A000E544/1433288371.png" \
        "./data/_yberbrowser/1629879429.png      /epoc32/winscw/c/private/A000E544/1629879429.png" \
        "./data/_yberbrowser/1648991282.png      /epoc32/winscw/c/private/A000E544/1648991282.png" \
        "./data/_yberbrowser/1709185409.png      /epoc32/winscw/c/private/A000E544/1709185409.png" \
        "./data/_yberbrowser/1752222667.png      /epoc32/winscw/c/private/A000E544/1752222667.png" \
        "./data/_yberbrowser/1949898887.png      /epoc32/winscw/c/private/A000E544/1949898887.png" \
        "./data/_yberbrowser/1956761810.png      /epoc32/winscw/c/private/A000E544/1956761810.png" \
        "./data/_yberbrowser/2101853025.png      /epoc32/winscw/c/private/A000E544/2101853025.png" \
        "./data/_yberbrowser/2228149477.png      /epoc32/winscw/c/private/A000E544/2228149477.png" \
        "./data/_yberbrowser/2381847146.png      /epoc32/winscw/c/private/A000E544/2381847146.png" \
        "./data/_yberbrowser/2461283667.png      /epoc32/winscw/c/private/A000E544/2461283667.png" \
        "./data/_yberbrowser/2562298915.png      /epoc32/winscw/c/private/A000E544/2562298915.png" \
        "./data/_yberbrowser/2600348047.png      /epoc32/winscw/c/private/A000E544/2600348047.png" \
        "./data/_yberbrowser/2681767302.png      /epoc32/winscw/c/private/A000E544/2681767302.png" \
        "./data/_yberbrowser/2690441205.png      /epoc32/winscw/c/private/A000E544/2690441205.png" \
        "./data/_yberbrowser/2733289833.png      /epoc32/winscw/c/private/A000E544/2733289833.png" \
        "./data/_yberbrowser/2742934577.png      /epoc32/winscw/c/private/A000E544/2742934577.png" \
        "./data/_yberbrowser/2934683436.png      /epoc32/winscw/c/private/A000E544/2934683436.png" \
        "./data/_yberbrowser/2979298446.png      /epoc32/winscw/c/private/A000E544/2979298446.png" \
        "./data/_yberbrowser/3195535666.png      /epoc32/winscw/c/private/A000E544/3195535666.png" \
        "./data/_yberbrowser/3362099564.png      /epoc32/winscw/c/private/A000E544/3362099564.png" \
        "./data/_yberbrowser/3365415225.png      /epoc32/winscw/c/private/A000E544/3365415225.png" \
        "./data/_yberbrowser/bookmarkstore.txt      /epoc32/winscw/c/private/A000E544/bookmarkstore.txt" \
        "./data/_yberbrowser/historystore.txt      /epoc32/winscw/c/private/A000E544/historystore.txt" \
}

target.path = $$BINDIR
INSTALLS += target
RESOURCES = yberbrowser.qrc

maemo5 {
    dotdesktop.files = data/yberbrowser-fremantle.desktop
    dotdesktop.path = $$DATADIR/applications/hildon
    INSTALLS += dotdesktop

    icons.files = data/icon/*
    icons.path = $$DATADIR/icons/hicolor
    INSTALLS += icons
}

HEADERS = \
  src/ApplicationWindow.h \
  src/ApplicationWindowHost.h \
  src/AutoSelectLineEdit.h \
  src/AutoSelectLineEdit_p.h \
  src/AutoScrollTest.h \
  src/BackingStoreVisualizerWidget.h \
  src/BookmarkStore.h \
  src/BrowsingView.h \
  src/CommonGestureRecognizer.h \
  src/CookieJar.h \
  src/EnvHttpProxyFactory.h \
  src/EventHelpers.h \
  src/FontFactory.h \
  src/Helpers.h \
  src/HistoryStore.h \
  src/HomeView.h \
  src/KeypadWidget.h \
  src/LinkSelectionItem.h \
  src/PannableTileContainer.h \
  src/PannableViewport.h \
  src/PopupView.h \
  src/ProgressWidget.h \
  src/ScrollbarItem.h \
  src/Settings.h \
  src/TileContainerWidget.h \
  src/TileItem.h \
  src/TileSelectionViewBase.h \
  src/ToolbarWidget.h \
  src/UrlItem.h \
  src/WebView.h \
  src/WebViewportItem.h \
  src/YberApplication.h

SOURCES = \
  src/AutoSelectLineEdit.cpp \
  src/AutoScrollTest.cpp \
  src/BackingStoreVisualizerWidget.cpp \
  src/BookmarkStore.cpp \
  src/BrowsingView.cpp \
  src/CommonGestureRecognizer.cpp \
  src/CookieJar.cpp \
  src/EnvHttpProxyFactory.cpp\
  src/EventHelpers.cpp \
  src/FontFactory.cpp \
  src/Helpers.cpp \
  src/HistoryStore.cpp \
  src/HomeView.cpp \
  src/KeypadWidget.cpp \
  src/LinkSelectionItem.cpp \
  src/PopupView.cpp \
  src/ProgressWidget.cpp \
  src/ScrollbarItem.cpp \
  src/TileContainerWidget.cpp \
  src/TileItem.cpp \
  src/TileSelectionViewBase.cpp \
  src/ToolbarWidget.cpp \
  src/UrlItem.cpp \
  src/WebView.cpp \
  src/WebViewportItem.cpp \
  src/YberApplication.cpp \
  src/main.cpp

HEADERS += \
  3rdparty/qabstractkineticscroller.h \
  3rdparty/qabstractkineticscroller_p.h \
  src/WebViewport.h

SOURCES += \
  src/PannableTileContainer.cpp \
  src/WebViewport.cpp \
  3rdparty/qabstractkineticscroller.cpp \

!meegotouch {
SOURCES += \
  src/PannableViewport.cpp \
  src/ApplicationWindowHost.cpp \
  src/ApplicationWindow.cpp
}

!enable_webkit2 {
HEADERS += src/WebPage.h
SOURCES += src/WebPage.cpp
}


# support for launcher see libdui/launcher.html
# the flags are used for all configurations, so we get the same flags
# during the development as for the target
# however, debugging -pie executables is not supported by gdb
!debug {
QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -pie -rdynamic
}
