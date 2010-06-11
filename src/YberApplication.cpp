/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "YberApplication.h"
#include "BrowsingView.h"
#include "Settings.h"
#include "Helpers.h"
#include "EnvHttpProxyFactory.h"
#include "ApplicationWindow.h"

#include <QUrl>
#include <QNetworkProxyFactory>
#include <QDebug>

YberApplication::YberApplication(int & argc, char ** argv)
    : YberApplicationBase(argc, argv)
    , appwin(0)
    , m_cookieJar(0)
{
    bool useSystemConf = true;

    QList<QNetworkProxy> proxylist = QNetworkProxyFactory::systemProxyForQuery();

    if (proxylist.count() == 1) {
        QNetworkProxy proxy = proxylist.first();
        if (proxy == QNetworkProxy::NoProxy || proxy == QNetworkProxy::DefaultProxy) {
            EnvHttpProxyFactory* pf = new EnvHttpProxyFactory();
            if (pf->initFromEnvironment()) {
                QNetworkProxyFactory::setApplicationProxyFactory(pf);
                useSystemConf = false;
            }
        }
    }
    if (useSystemConf)
        QNetworkProxyFactory::setUseSystemConfiguration(true);
}


YberApplication::~YberApplication()
{
    delete m_cookieJar;
}

void YberApplication::start()
{
    if (!appwin) {
        appwin = new ApplicationWindow();
        if (Settings::instance()->isFullScreen())
            appwin->showFullScreen();
        else
            appwin->show();
    }
}

void YberApplication::createMainView(const QUrl& url)
{
    BrowsingView* page = new BrowsingView(*this);

#if USE_DUI
    page->appear(appwin, DuiSceneWindow::DestroyWhenDone);
#else
    page->appear(appwin);
#endif

    if (!url.isEmpty())
        page->load(url);
}

CookieJar* YberApplication::cookieJar() const
{
    if (!m_cookieJar)
        m_cookieJar = new CookieJar;
    return m_cookieJar;
}
