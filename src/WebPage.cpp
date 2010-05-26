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

#include "WebPage.h"
#include "BrowsingView.h"
#include "WebView.h"
#include "YberApplication.h"

#include <QDebug>

WebPage::WebPage(QObject* parent, BrowsingView* ownerView)
    : QWebPage(parent)
    , m_ownerView(ownerView)
{
    CookieJar* jar = YberApplication::instance()->cookieJar();
    // setCookieJar changes the parent of the passed jar ;(
    // So we need to preserve it
    QObject* oldParent = jar->parent();
    networkAccessManager()->setCookieJar(jar);
    jar->setParent(oldParent);
}

QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    if (!m_ownerView)
        return 0;

    WebView* webView = m_ownerView->newWindow();

    if (!webView)
        return 0;

    return webView->page();
}

QString WebPage::userAgentForUrl(const QUrl&) const
{
    static QString userAgent = QWebPage::userAgentForUrl(QUrl())
#if !defined(Q_OS_SYMBIAN) && !defined(Q_WS_MAEMO_5)
        .replace("Safari", "Mobile Safari")
#endif
        // NOTE: For testing purposed we want to receive pages
        // created for the iPhone or Android. Yberbrowser is not
        // known enough for people serving us these pages.
        .replace("Linux", "Linux, like Android");
    return userAgent;
}
