/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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

#include "WebViewportItem.h"
#include "EventHelpers.h"
#include "WebView.h"

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
#include <QtGlobal>

#ifndef QTWEBKIT_VERSION_CHECK
#define QTWEBKIT_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#endif

namespace {
const int s_defaultPreferredWidth = 800;
const int s_defaultPreferredHeight = 480;
const qreal s_minZoomScale = .01; // arbitrary
const qreal s_maxZoomScale = 10.; // arbitrary
#if !USE_WEBKIT2
const int s_minDoubleClickZoomTargetWidth = 100; // in document coords, aka CSS pixels
#endif
const int s_zoomCommitTimerDurationMS = 100;
const qreal s_zoomableContentMinWidth = 300.;
const qreal s_zoomRectAdjustHeight = 5.;
const qreal s_zoomRectAdjustWidth = 5.;
const qreal s_dpiAdjustmentFactor = 1.5;
}

/*!
 \class WebViewportItem graphics item representing the whole web page
 responsible for zooming the document when the item size changes

 \WebViewportItem is a \GraphicsItem which is the size of the web page.
 it scales the underlying \QGraphicsWebView to its own size.
 */

/*!
 * \view ownership transfer
 */
WebViewportItem::WebViewportItem(QGraphicsWidget* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_webView(0)
    , m_zoomCommitTimer(this)
    , m_resizeMode(WebViewportItem::ContentResizePreservesWidth)
    , m_zoomPos(0, 0)
{
#if !defined(ENABLE_PAINT_DEBUG)
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);

    connect(&m_zoomCommitTimer, SIGNAL(timeout()), this, SLOT(commitZoom()));
    m_zoomCommitTimer.setSingleShot(true);
}

WebViewportItem::~WebViewportItem()
{
}

void WebViewportItem::setWebView(WebView* webView)
{
    m_zoomCommitTimer.stop();

    if (m_webView == webView)
        return;

    if (m_webView) {
        disconnectWebViewSignals();
        WebView* oldWebView = m_webView;
        m_webView = 0;
        oldWebView->setParent(0);
    }

    m_webView = webView;
    m_webView->setParentItem(this);
    m_webView->setAttribute(Qt::WA_OpaquePaintEvent, true);

#if USE_WEBKIT2
    updatePreferredSize();
#else
    m_webView->setResizesToContents(true);
#endif

    connectWebViewSignals();
}

void WebViewportItem::connectWebViewSignals()
{
#if USE_WEBKIT2
    connect(m_webView->page(), SIGNAL(zoomRectReceived(const QRect&)), this, SLOT(zoomRectReceived(const QRect&)));
    connect(m_webView->page(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(webViewContentsSizeChanged(const QSize&)));
#else
    connect(m_webView->page()->mainFrame(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(webViewContentsSizeChanged(const QSize&)));
#if QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 1, 0)
    connect(m_webView->page(), SIGNAL(viewportChangeRequested(const QWebPage::ViewportHints&)), this, SLOT(adjustViewport(const QWebPage::ViewportHints&)));
#endif
#endif
}

void WebViewportItem::disconnectWebViewSignals()
{
#if USE_WEBKIT2
    disconnect(m_webView->page(), SIGNAL(zoomRectReceived(const QRect&)), this, SLOT(zoomRectReceived(const QRect&)));
    disconnect(m_webView->page(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(webViewContentsSizeChanged(const QSize&)));
#else
    disconnect(m_webView->page()->mainFrame(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(webViewContentsSizeChanged(const QSize&)));
#if QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 1, 0)
    disconnect(m_webView->page(), SIGNAL(viewportChangeRequested(const QWebPage::ViewportHints&)), this, SLOT(adjustViewport(const QWebPage::ViewportHints&)));
#endif
#endif
}


void WebViewportItem::commitZoom()
{
#if !USE_WEBKIT2
    m_webView->setTiledBackingStoreFrozen(false);
#endif
    m_zoomCommitTimer.stop();
}

/*!
\fn void WebViewportItem::contentsSizeChangeCausedResize()
This signal is emitted when contents size has changed and vieport item is resized due to this
*/
void WebViewportItem::webViewContentsSizeChanged(const QSize& newContentsSize)
{
#if USE_WEBKIT2
    m_webView->setGeometry(QRect(0, 0, newContentsSize.width(), newContentsSize.height()));
#endif
    QSizeF currentSize = size();
    qreal targetScale = zoomScale();

    m_contentSize = newContentsSize;

    switch(m_resizeMode) {
    case WebViewportItem::ContentResizePreservesWidth:
        targetScale = currentSize.width() / newContentsSize.width();
        break;
    case WebViewportItem::ContentResizePreservesHeight:
        targetScale = currentSize.height() / newContentsSize.height();
        break;
    case WebViewportItem::ContentResizePreservesScale:
        break;
    }

    QSizeF scaledSize = newContentsSize * targetScale;

    resize(scaledSize);

    // fixme: emit only when size changed (or change signal name)
    emit contentsSizeChangeCausedResize();
}

#if defined(ENABLE_PAINT_DEBUG)
void WebViewportItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    static int n = 0;
    ++n;
    painter->save();
    painter->setPen(Qt::green);
    painter->setBrush(Qt::green);
    painter->fillRect(option->exposedRect.toRect(), n % 2 ? Qt::green : Qt::darkGreen );
    painter->restore();
    QGraphicsWidget::paint(painter, option, widget);
}
#endif

QSize WebViewportItem::contentsSize() const
{
#if USE_WEBKIT2
    return m_contentSize;
#else
    return m_webView->page()->mainFrame()->contentsSize();
#endif
}

void WebViewportItem::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);
    setZoomScale(size().width() / contentsSize().width());
}

void WebViewportItem::disableContentUpdates()
{
#if !USE_WEBKIT2    
    m_webView->setTiledBackingStoreFrozen(true);
#endif
}

void WebViewportItem::enableContentUpdates()
{
#if !USE_WEBKIT2    
    m_webView->setTiledBackingStoreFrozen(false);
#endif
    // FIXME what to do with this?
//    m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
}

#if USE_WEBKIT2
void WebViewportItem::updatePreferredSize()
{
    // FIXME: we have bug in QtWebKit API when tileCacheEnabled is true.
    // this causes viewport not to reset between the page loads.
    // Thus, we need to update viewport manually until we have fix for this.

//    m_webView->page()->setPreferredContentsSize(QSize(s_defaultPreferredWidth, s_defaultPreferredHeight));
    resize(contentsSize());
}
#elif QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 1, 0)
/*!
    This method is called before the first layout of the contents and might
    come with viewport data requested by the page via the viewport meta tag.
*/
void WebViewportItem::adjustViewport(const QWebPage::ViewportHints& viewportInfo)
{
    // for an explanation of pixelScale look at:
    // http://hacks.mozilla.org/2010/05/upcoming-changes-to-the-viewport-meta-tag-for-firefox-mobile/
    qreal pixelScale = s_dpiAdjustmentFactor;

    QSize viewportSize = QSize(s_defaultPreferredWidth, s_defaultPreferredHeight);

    if (viewportInfo.size().width() > 0)
        viewportSize.setWidth(viewportInfo.size().width());
    if (viewportInfo.size().height() > 0)
        viewportSize.setHeight(viewportInfo.size().height());

    m_webView->page()->setPreferredContentsSize(viewportSize / pixelScale);

    // FIXME we should start using the scale range at some point
    // viewportInfo.minimumScaleFactor() and viewportInfor.maximumScaleFactor()
    if (viewportInfo.initialScaleFactor() > 0) {
        setResizeMode(WebViewport::ContentResizePreservesScale);
        setZoomScale(viewportInfo.initialScaleFactor() * pixelScale, /* immediate */ true);
    } else {
        setResizeMode(WebViewport::ContentResizePreservesWidth);
        setZoomScale(1.0 * pixelScale, /* immediate */ true);
    }
}
#endif

void WebViewportItem::zoomRectReceived(const QRect& zoomRect)
{
    QPointF webp = m_webView->mapFromParent(m_zoomPos);
    QRectF er = zoomRect;
    er.adjust(-s_zoomRectAdjustWidth, -s_zoomRectAdjustHeight, s_zoomRectAdjustWidth, s_zoomRectAdjustHeight);
    qreal overMinWidth = er.width() - s_zoomableContentMinWidth;
    if (overMinWidth < 0)
        er.adjust(overMinWidth / 2, 0, -overMinWidth / 2, 0);
    webp.setX(er.x());
    QRectF res(webp, er.size());
    QRectF finalRect(m_webView->mapToParent(res.topLeft()),m_webView->mapToParent(res.bottomRight()));
    emit zoomRectForPointReceived(m_zoomPos, finalRect);
    m_zoomPos = QPoint(0, 0);
}

/*!  Returns a rectangle of a content element containing point \p in
  current item coordinates.
*/
void WebViewportItem::findZoomableRectForPoint(const QPointF& p)
{
    // FIXME check if m_zoomPos is not null -> pending zoom operation
    //if (!m_zoomPos.isNull())
    m_zoomPos = p.toPoint();
#if USE_WEBKIT2
    m_webView->page()->requestZoomRect(m_webView->mapFromParent(p).toPoint());
#else
    QPointF webp = m_webView->mapFromParent(p);

    QWebHitTestResult r = m_webView->page()->mainFrame()->hitTestContent(webp.toPoint());
    QWebElement e = r.enclosingBlockElement();

    while (!e.isNull() && e.geometry().width() < s_minDoubleClickZoomTargetWidth) {
        e = e.parent();
    }
    if (!e.isNull())
        zoomRectReceived(e.geometry());
#endif
}

qreal WebViewportItem::zoomScale() const
{
    if (!m_webView)
        return 1.;

    return m_webView->scale();
}

void WebViewportItem::setZoomScale(qreal value, bool commitInstantly)
{
    if (!commitInstantly) {
        disableContentUpdates();
    }

    value = qBound(s_minZoomScale, value, s_maxZoomScale);
    qreal curZoomScale = zoomScale();
    if (value != curZoomScale) {
        // fixme
        // QPointF p = m_webView->pos();
        m_webView->setScale(value);
        //p *= value / curZoomScale;
        //setPos(p);
    }
    if (commitInstantly)
        commitZoom();
    else
        m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
}

void WebViewportItem::setResizeMode(WebViewportItem::ResizeMode mode)
{
    m_resizeMode = mode;
}
