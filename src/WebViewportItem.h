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

#ifndef WebViewportItem_h_
#define WebViewportItem_h_

#include "yberconfig.h"

#include <QGraphicsWidget>
#include <QTimer>
#if !USE_WEBKIT2
#include "qwebpage.h"
#endif

class WebView;

class WebViewportItem : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal zoomScale READ zoomScale WRITE setZoomScale)

public:
    WebViewportItem(QGraphicsWidget* parent = 0, Qt::WindowFlags wFlags = 0);
    ~WebViewportItem();

    void setWebView(WebView* webView);
    WebView* webView() const { return m_webView; }

    QSize contentsSize() const;

    void disableContentUpdates();
    void enableContentUpdates();

    void findZoomableRectForPoint(const QPointF&);

    void setZoomScale(qreal, bool=false);
    qreal zoomScale() const;

    enum ResizeMode {
        ContentResizePreservesScale,
        ContentResizePreservesWidth,
        ContentResizePreservesHeight
    };

    void setResizeMode(ResizeMode);
    ResizeMode resizeMode() const { return m_resizeMode; }

public Q_SLOTS:
    void commitZoom();
#if QTWEBKIT_VERSION >= 0x020100 && !USE_WEBKIT2
    void adjustViewport();
#endif

Q_SIGNALS:
    void contentsSizeChangeCausedResize();
    void zoomRectForPointReceived(const QPointF&, const QRectF&);

protected:
    void resizeEvent(QGraphicsSceneResizeEvent* event);

#if defined(ENABLE_PAINT_DEBUG)
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
#endif

protected Q_SLOTS:
    void webViewContentsSizeChanged(const QSize &size);
    void zoomRectReceived(const QRect& zoomRect);

private:
    Q_DISABLE_COPY(WebViewportItem)
#if USE_WEBKIT2
    void updatePreferredSize();
#endif
    void connectWebViewSignals();
    void disconnectWebViewSignals();

    WebView* m_webView;
    QTimer m_zoomCommitTimer;
    ResizeMode m_resizeMode;
    QPointF m_zoomPos;
    QSize m_contentSize;
};

#endif
