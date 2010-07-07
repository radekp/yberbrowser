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

YberApplication::YberApplication()
    : m_appwin(0)
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

void YberApplication::startWithWindow(ApplicationWindow* appwin)
{
    Q_ASSERT(!m_appwin);

    if (!m_appwin) {
        m_appwin = appwin;
        if (Settings::instance()->isFullScreen())
            m_appwin->showFullScreen();
        else
            m_appwin->show();
    }

}

void YberApplication::createMainView(const QUrl& url)
{
    BrowsingView* page = new BrowsingView();

#if USE_MEEGOTOUCH
    page->setAutoMarginsForComponentsEnabled(true);
    if (Settings::instance()->isFullScreen()) {
        page->setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
    }
    page->appear(m_appwin, MSceneWindow::DestroyWhenDone);
    page->setPos(0, 30);
#else
    page->appear(m_appwin);
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

YberApplication* YberApplication::instance()
{
    static YberApplication* self = 0;
    if (!self)
        self = new YberApplication;
    return self;
}

