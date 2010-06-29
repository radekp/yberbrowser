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

#include "WebView.h"
#if USE_WEBKIT2
#include "WebKit2/WKFrame.h"
#endif
#include "YberApplication.h"

#if USE_WEBKIT2
static QWKPage* createNewPageCallback(QWKPage* page)
{
    // FIXME same page is used for now
    return page;
}

WebView::WebView(WKPageNamespaceRef namespaceRef, QGraphicsItem* parent)
    : QWKGraphicsWidget(namespaceRef, QWKGraphicsWidget::Tiled, parent)
    , m_fpsTicks(0)
{
    setCookieJar();
    applyPageSettings();
    page()->setCreateNewPageFunction(createNewPageCallback);
}
#else
WebView::WebView(QGraphicsItem* parent)
    : QGraphicsWebView(parent)
    , m_fpsTicks(0)
{
    setCookieJar();
    applyPageSettings();
}
#endif

void WebView::paint(QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* w)
{
    m_fpsTicks++;
#if USE_WEBKIT2
   QWKGraphicsWidget::paint(p, option, w);
#else
    QGraphicsWebView::paint(p, option, w);
#endif
}

void WebView::applyPageSettings()
{
    page()->setProperty("_q_TiledBackingStoreTileSize", QSize(256, 256));
    page()->setProperty("_q_TiledBackingStoreTileCreationDelay", 25);
    page()->setProperty("_q_TiledBackingStoreCoverAreaMultiplier", QSizeF(1.5, 1.5));
    page()->setProperty("_q_TiledBackingStoreKeepAreaMultiplier", QSizeF(2., 2.5));
}

void WebView::setCookieJar()
{
#if !USE_WEBKIT2
    CookieJar* jar = YberApplication::instance()->cookieJar();
    // setCookieJar changes the parent of the passed jar ;(
    // So we need to preserve it
    QObject* oldParent = jar->parent();
    page()->networkAccessManager()->setCookieJar(jar);
    jar->setParent(oldParent);
#endif
}

