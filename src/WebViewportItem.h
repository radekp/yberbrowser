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


#include <QGraphicsWidget>
#include <QPropertyAnimation>
#include <QGraphicsItemAnimation>
#include <QTimer>
#include <QTimeLine>

class QGraphicsWebView;

class WebViewportItem : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal zoomScale READ zoomScale WRITE setZoomScale)

public:
    WebViewportItem(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);

    qreal zoomScale() const;
    void setZoomScale(qreal value, bool commitInstantly = false);

    void animateZoomScaleTo(qreal targetZoomScale);
    void setWebView(QGraphicsWebView* view);
    QGraphicsWebView* webView();

    void resetState(bool resetZoom);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void wheelEvent(QGraphicsSceneWheelEvent * event);

#if defined(ENABLE_PAINT_DEBUG)
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
#endif
    bool sceneEventFilter(QGraphicsItem *i, QEvent *e);

protected Q_SLOTS:
    void commitZoom();
    void panAnimStateChanged(QTimeLine::State newState);

private:
    bool sendWheelEventFromChild(QGraphicsSceneWheelEvent *event);
    bool sendMouseEventFromChild(QGraphicsSceneMouseEvent *event);
    void transferAnimStateToView();
    void resetZoomAnim();
    void startZoomAnimTo(const QPointF& targetPoint, qreal targetScale);

    bool isZoomedIn() const;

    void clearDelayedPress();
    void captureDelayedPress(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsWebView* m_webView;
    bool m_isDragging;
    qreal m_zoomScale;
    QPointF m_dragStartPos;
    QPointF m_itemPos;
    QGraphicsItemAnimation m_zoomAnim;
    QTimer m_zoomCommitTimer;

    QGraphicsSceneMouseEvent* m_delayedPressEvent;
    QGraphicsItem* m_delayedPressTarget;
    QBasicTimer m_delayedPressTimer;
    int m_pressDelay;

};

#endif
