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

#include "YberApplication.h"
#include "Settings.h"
#include "Helpers.h"

#include <QDebug>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QNetworkRequest>
#include <QTextStream>
#include <QVector>
#include <QtGui>
#include <cstdio>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>
#if USE_MEEGOTOUCH
#include <MApplication>
#include <MApplicationPage>
#include <MComponentCache>
#endif

#if QTOPIA
#include <QtopiaApplication>
#endif

#include "ApplicationWindow.h"

void usage(const char* name);


#if USE_MEEGOTOUCH
M_EXPORT
#endif
int main(int argc, char** argv)
{
 #if USE_MEEGOTOUCH
    MApplication* app = MComponentCache::mApplication(argc, argv);
    MApplicationWindow* window = MComponentCache::mApplicationWindow();
#elif QTOPIA
    QtopiaApplication* app = new QtopiaApplication(argc, argv);
    ApplicationWindow* window = new ApplicationWindow();
#else
    QApplication* app = new QApplication(argc, argv);
    ApplicationWindow* window = new ApplicationWindow();
#endif

    app->setApplicationName("yberbrowser");

    QString privPath;
#ifdef Q_OS_SYMBIAN
    privPath = QDir::toNativeSeparators(QApplication::applicationDirPath()) + QString("\\");
#else
    privPath = QString("%1/.%2/").arg(QDir::homePath()).arg(QCoreApplication::applicationName());
    QDir privDir(privPath);
    if (!privDir.exists()) {
        privDir.mkpath(privPath);
    }
#endif
    Settings* settings = Settings::instance();

    settings->setPrivatePath(privPath);

    QWebSettings::setObjectCacheCapacities((16 * 1024 * 1024) / 8, (16 * 1024 * 1024) / 8, 16 * 1024 * 1024);
    QWebSettings::setMaximumPagesInCache(4);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    QWebSettings::enablePersistentStorage(settings->privatePath());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::FrameFlatteningEnabled, true);

    QStringList args = app->arguments();

    settings->enableTileCache(true);

    bool gotFlag = true;
    while (gotFlag) {
        if (args.count() > 1) {
            if (args.at(1) == "-w") {
                settings->setIsFullScreen(false);
                args.removeAt(1);
            } else if (args.at(1) == "-t") {
                settings->enableToolbar(false);
                args.removeAt(1);
            } else if (args.at(1) == "-g") {
                settings->setUseGL(true);
                args.removeAt(1);
            } else if (args.at(1) == "-c") {
                settings->enableTileCache(false);
                args.removeAt(1);
            } else if (args.at(1) == "-v") {
                settings->enableTileVisualization(true);
                args.removeAt(1);
            } else if (args.at(1) == "-a") {
                settings->enableAutoComplete(false);
                args.removeAt(1);
            } else if (args.at(1) == "-f") {
                settings->enableFPS(true);
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

    QString url;
    if (args.count() > 1)
        url = args.at(1);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::TiledBackingStoreEnabled, settings->tileCacheEnabled());


    YberApplication::instance()->startWithWindow(window);
    YberApplication::instance()->createMainView(urlFromUserInput(url));

    for (int i = 2; i < args.count(); i++) {
        if (args.at(i) != "-software")
            YberApplication::instance()->createMainView(urlFromUserInput(args.at(i)));
    }

#if QTOPIA
    window->showMaximized();
#endif
    int retval = app->exec();

#if !defined(NDEBUG)
    delete app;
#endif
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
    s << " -v enable tile visualization" << endl;
    s << " -f show fps counter" << endl;
    s << " -a disable url autocomplete" << endl;
    s << " -h|-?|--help help" << endl;
    s << endl;
    s << " use http_proxy env var to set http proxy" << endl;
}

