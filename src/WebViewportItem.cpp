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
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>
#include <QtGlobal>

namespace {
const int s_defaultPreferredWidth = 1024;
const int s_defaultPreferredHeight = 768;
const qreal s_minZoomScale = .01; // arbitrary
const qreal s_maxZoomScale = 10.; // arbitrary
const int s_minDoubleClickZoomTargetWidth = 100; // in document coords, aka CSS pixels
const int s_zoomCommitTimerDurationMS = 100;
const qreal s_zoomableContentMinWidth = 300.;
const qreal s_zoomRectAdjustHeight = 5.;
const qreal s_zoomRectAdjustWidth = 5.;
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
    , m_resizeMode(WebViewportItem::ResizeWidgetHeightToContent)
{
#if !defined(ENABLE_PAINT_DEBUG)
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);

    setFiltersChildEvents(true);
    connect(&m_zoomCommitTimer, SIGNAL(timeout()), this, SLOT(commitZoom()));
    m_zoomCommitTimer.setSingleShot(true);
}

WebViewportItem::~WebViewportItem()
{
}

void WebViewportItem::setWebView(QGraphicsWebView* webView) 
{ 
    m_webView = webView; 

    m_webView->setResizesToContents(true);

    m_webView->setParentItem(this);
    m_webView->setAttribute(Qt::WA_OpaquePaintEvent, true);

    updatePreferredSize();

    connect(m_webView->page()->mainFrame(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(webViewContentsSizeChanged(const QSize&)));
}

void WebViewportItem::commitZoom()
{
    m_webView->setTiledBackingStoreFrozen(false);
    m_zoomCommitTimer.stop();
}

/*!
\fn void WebViewportItem::contentsSizeChangeCausedResize()
This signal is emitted when contents size has changed and vieport item is resized due to this
*/
void WebViewportItem::webViewContentsSizeChanged(const QSize& sz)
{
    Q_UNUSED(sz);
    qreal scale = zoomScale();
    if (m_resizeMode == WebViewportItem::ResizeWidgetHeightToContent) {
        scale = size().width() / contentsSize().width();
    }
    resize(contentsSize() * scale);
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
    return m_webView->page()->mainFrame()->contentsSize();
}

void WebViewportItem::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);
    setZoomScale(size().width() / contentsSize().width());
}

void WebViewportItem::disableContentUpdates()
{
    m_webView->setTiledBackingStoreFrozen(true);
}

void WebViewportItem::enableContentUpdates()
{
    m_webView->setTiledBackingStoreFrozen(false);
    // FIXME what to do with this?
//    m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
}

void WebViewportItem::updatePreferredSize()
{
    // FIXME: we have bug in QtWebKit API when tileCacheEnabled is true.
    // this causes viewport not to reset between the page loads.
    // Thus, we need to update viewport manually until we have fix for this.

    m_webView->page()->setPreferredContentsSize(QSize(s_defaultPreferredWidth, s_defaultPreferredHeight));
    resize(contentsSize());
}

/*!  Returns a rectangle of a content element containing point \p in
  current item coordinates.
*/
QRectF WebViewportItem::findZoomableRectForPoint(const QPointF& p)
{
    QPointF webp = m_webView->mapFromParent(p);

    QWebHitTestResult r = m_webView->page()->mainFrame()->hitTestContent(webp.toPoint());
    QWebElement e = r.enclosingBlockElement();

    while (!e.isNull() && e.geometry().width() < s_minDoubleClickZoomTargetWidth) {
        e = e.parent();
    }
    if (!e.isNull()) {
        QRectF er = e.geometry();
        er.adjust(-s_zoomRectAdjustWidth, -s_zoomRectAdjustHeight, s_zoomRectAdjustWidth, s_zoomRectAdjustHeight);
        qreal overMinWidth = er.width() - s_zoomableContentMinWidth;
        if (overMinWidth < 0)
            er.adjust(overMinWidth / 2, 0, -overMinWidth / 2, 0);
        webp.setX(er.x());
        QRectF res(webp, er.size());
        return QRectF(m_webView->mapToParent(res.topLeft()),
                      m_webView->mapToParent(res.bottomRight()));
    }
    return QRectF();
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

void WebViewportItem::setGeometry(const QRectF&r)
{
    QGraphicsWidget::setGeometry(r);
}

void WebViewportItem::setResizeMode(WebViewportItem::ResizeMode mode)
{
    m_resizeMode = mode;
}
