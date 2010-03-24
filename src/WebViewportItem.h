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

class QGraphicsWebView;

class WebViewportItem : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal zoomScale READ zoomScale WRITE setZoomScale)

public:
    WebViewportItem(QGraphicsWebView*, QGraphicsWidget* parent = 0, Qt::WindowFlags wFlags = 0);
    ~WebViewportItem();

    QGraphicsWebView* webView();

    QSize contentsSize() const;

    void disableContentUpdates();
    void enableContentUpdates();

    QRectF findZoomableRectForPoint(const QPointF&);

    void setZoomScale(qreal, bool=false);
    qreal zoomScale() const;

Q_SIGNALS:
    void contentsSizeChangeCausedResize();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent* event);

#if defined(ENABLE_PAINT_DEBUG)
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
#endif

protected Q_SLOTS:
    void webViewContentsSizeChanged(const QSize &size);
    void commitZoom();
    
private:
    Q_DISABLE_COPY(WebViewportItem);
    void updatePreferredSize();

    QGraphicsWebView* m_webView;
    QTimer m_zoomCommitTimer;

};

#endif
