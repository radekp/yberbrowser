QT +=  webkit network xml


TEMPLATE=app
TARGET=yberbrowser

CONFIG+=qtopia
DEFINES+=QTOPIA

#DEFINES+=USE_WEBKIT2

# I18n info
STRING_LANGUAGE=en_US
LANGUAGES=en_US

RESOURCES = yberbrowser.qrc

# Package info
pkg [
    name=yberbrowser
    desc="Fast and touch friendly web browser"
    license=LGPL
    version=1.0
    maintainer="Radek Polak <psonek2@seznam.cz>"
]

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
  src/YberApplication.h \
  3rdparty/qabstractkineticscroller.h \
  3rdparty/qabstractkineticscroller_p.h \
  src/WebViewport.h \
  src/WebPage.h

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
  src/main.cpp \
  src/PannableTileContainer.cpp \
  src/WebViewport.cpp \
  3rdparty/qabstractkineticscroller.cpp \
  src/PannableViewport.cpp \
  src/ApplicationWindowHost.cpp \
  src/ApplicationWindow.cpp \
  src/WebPage.cpp

# Install rules
target [
    hint=sxe
    domain=untrusted
]

desktop [
    hint=desktop
    files=data/yberbrowser.desktop
    path=/apps/Applications
]

pics [
    hint=pics
    files=data/yberbrowser.png
    path=/pics/yberbrowser
]
