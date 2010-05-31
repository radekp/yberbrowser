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

MOC_DIR     = $$PWD/.moc
OBJECTS_DIR = $$PWD/.obj
RCC_DIR     = $$PWD/.rcc

dui {
    CONFIG+=dui
    DEFINES+=USE_DUI=1
} else {
    DEFINES+=USE_DUI=0
}

*-g++*: QMAKE_CXXFLAGS += -Werror

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

enable_engine_thread {
    DEFINES+=ENABLE_ENGINE_THREAD=1
} else {
    DEFINES+=ENABLE_ENGINE_THREAD=0
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
  src/WebPage.h \
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
  src/WebPage.cpp \
  src/WebView.cpp \
  src/WebViewportItem.cpp \
  src/YberApplication.cpp \
  src/main.cpp

!dui {
HEADERS += \
  3rdparty/qabstractkineticscroller.h \
  3rdparty/qabstractkineticscroller_p.h \
  src/WebViewport.h

SOURCES += \
  src/ApplicationWindowHost.cpp \
  src/PannableTileContainer.cpp \
  src/PannableViewport.cpp \
  src/WebViewport.cpp \
  3rdparty/qabstractkineticscroller.cpp \
  src/ApplicationWindow.cpp

}


