TEMPLATE = app
SOURCES += main.cpp
CONFIG -= app_bundle

QT += network
macx:QT+=xml

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
