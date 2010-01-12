/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2009 Kenneth Christiansen <kenneth@webkit.org>
 * Copyright (C) 2009 Antonio Gomes <antonio.gomes@openbossa.org>
 * Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QDebug>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QNetworkRequest>
#include <QTextStream>
#include <QVector>
#include <QtGui>
#include <QtNetwork/QNetworkProxy>
#include <cstdio>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>
#include <QGLWidget>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "MainWindow.h"
#include "MainView.h"

QNetworkProxy* g_globalProxy;

void usage(const char* name);

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QUrl proxyUrl = MainWindow::urlFromUserInput(qgetenv("http_proxy"));
    if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
        int proxyPort = (proxyUrl.port() > 0) ? proxyUrl.port() : 8080;
        g_globalProxy = new QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyPort);
    }

    QString url = QString("file://%1/%2").arg(QDir::homePath()).arg(QLatin1String("index.html"));

    app.setApplicationName("yberbrowser");

    QWebSettings::setObjectCacheCapacities((16 * 1024 * 1024) / 8, (16 * 1024 * 1024) / 8, 16 * 1024 * 1024);
    QWebSettings::setMaximumPagesInCache(4);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, false);

    QStringList args = app.arguments();

    bool disableToolbar = false;
    bool useGL = false;

#if WEBKIT_SUPPORTS_TILE_CACHE
    bool disableTiling = false;
#else
    bool disableTiling = true;
#endif

    bool noFullscreen = false;
    bool gotFlag = true;
    while (gotFlag) {
        if (args.count() > 1) {
            if (args.at(1) == "-w") {
                noFullscreen = true;
                args.removeAt(1);
            } else if (args.at(1) == "-t") {
                disableToolbar = true;
                args.removeAt(1);
            } else if (args.at(1) == "-g") {
                useGL = true;
                args.removeAt(1);
            } else if (args.at(1) == "-c") {
                disableTiling = true;
                args.removeAt(1);
            } else if (args.at(1) == "-?" || args.at(1) == "-h" || args.at(1) == "--help") {
                usage(argv[0]);
                return EXIT_SUCCESS;
            } else {
                gotFlag = false;
            }
        } else {
            gotFlag = false;
        }
    }

    if (args.count() > 1)
        url = args.at(1);

    MainWindow* window = new MainWindow(g_globalProxy, Settings(disableToolbar, disableTiling, useGL));
    window->load(url);
    if (noFullscreen)
        window->show();
    else
        window->showFullScreen();

    for (int i = 2; i < args.count(); i++) {
        window->newWindow(args.at(i));
        if (noFullscreen)
            window->show();
        else
            window->showFullScreen();
    }

    int retval = app.exec();


    delete g_globalProxy;
    return retval;
}


void usage(const char* name) 
{
    QTextStream s(stderr);
    s << "usage: " << name << " [options] url" << endl;
    s << " -w disable fullscreen" << endl;
    s << " -t disable toolbar" << endl;
    s << " -g use glwidget as qgv viewport" << endl;
    s << " -c disable tile cache" << endl;
    s << " -h|-?|--help help" << endl;
    s << endl;
    s << " use http_proxy env var to set http proxy" << endl;
}

