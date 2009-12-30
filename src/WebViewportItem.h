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

#include <QPropertyAnimation>
#include <QTimer>

class MainView;

#define USE_RECTITEM 0
#if USE_RECTITEM
#include <QGraphicsRectItem>

class WebViewportItem : public QGraphicsObject, public virtual QGraphicsRectItem
#else

#include <QGraphicsWidget>
class WebViewportItem : public QGraphicsWidget
#endif
{
    Q_OBJECT
    Q_PROPERTY(qreal zoomScale READ zoomScale WRITE setZoomScale)

public:
#if USE_RECTITEM
    WebViewportItem(MainView* owner, const QRectF& rect, QGraphicsItem* parent = 0);
#else
    WebViewportItem(MainView* owner, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
#endif

    qreal zoomScale();
    void setZoomScale(qreal value);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void wheelEvent(QGraphicsSceneWheelEvent * event);
    bool sceneEvent(QEvent *event);

protected Q_SLOTS:
    void commitZoom();

private:
    qreal m_zoomScale;
    bool m_isDragging;
    MainView* m_owner;
    QPointF m_dragPos;
    QPointF m_offset;
    int m_zoomLevel;
    QPropertyAnimation m_zoomAnim;
    QTimer m_zoomCommitTimer;
    static int s_zoomValues[];
};

#endif
