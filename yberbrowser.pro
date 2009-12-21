TEMPLATE = app
SOURCES += main.cpp
CONFIG -= app_bundle

isEmpty(PREFIX) {
    PREFIX = /usr
}
DATADIR = $$PREFIX/share
BINDIR = $$PREFIX/bin


QT += network opengl
macx:QT+=xml

disable_tile_cache {
    DEFINES+=DISABLE_TILE_CACHE=1
}
default_webkit {
    QT += webkit
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += QtWebKit-custom
}

symbian {
    TARGET.UID3 = 0xA000E544
    TARGET.CAPABILITY = ReadUserData WriteUserData NetworkServices
}

target.path = $$BINDIR

INSTALLS += target
