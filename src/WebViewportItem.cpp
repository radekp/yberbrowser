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

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QNetworkRequest>
#include <QTextStream>
#include <QVector>
#include <QtGui>
#include <QtNetwork/QNetworkProxy>
#include <cstdio>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>
#include <QGLWidget>
#include <QtGlobal>

#include "WebViewportItem.h"
#include "MainView.h"

int WebViewportItem::s_zoomValues[] = {30, 50, 67, 80, 90, 100, 110, 120, 133, 150, 170, 200, 240, 300};

#if USE_RECTITEM
WebViewportItem::WebViewportItem(MainView* owner, const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)
#else
WebViewportItem::WebViewportItem(MainView* owner, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
#endif
    , m_zoomScale(1.)
    , m_isDragging(false)
    , m_owner(owner)
    , m_zoomLevel(5)
    , m_zoomAnim(this, "zoomScale")
{
    m_zoomAnim.setDuration(300);
    connect(&m_zoomCommitTimer, SIGNAL(timeout()), this, SLOT(commitZoom()));
    m_zoomCommitTimer.setSingleShot(true);

}

void WebViewportItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_isDragging = true;
    m_dragPos = event->pos();

}

void WebViewportItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_isDragging = false;
    m_offset += event->pos() - m_dragPos;

}

void WebViewportItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_isDragging)
        m_owner->move(m_offset + (event->pos() - m_dragPos));
}

qreal WebViewportItem::zoomScale()
{
    return m_zoomScale;
}

void WebViewportItem::setZoomScale(qreal value)
{
    m_zoomScale = value;
    m_owner->zoom(m_zoomScale);
    m_zoomCommitTimer.start(500);
}

void WebViewportItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    int adv = event->delta() / (15*8);
    m_zoomLevel = qBound(0, m_zoomLevel + adv, int(sizeof(s_zoomValues) / sizeof(int)) - 1);

    m_zoomAnim.stop();
    m_zoomAnim.setStartValue(m_zoomScale);
    m_zoomAnim.setEndValue(s_zoomValues[m_zoomLevel] / 100.);
    m_zoomAnim.start();
}

bool WebViewportItem::sceneEvent(QEvent *event)
{
    //qDebug() << __PRETTY_FUNCTION__;
    return QGraphicsItem::sceneEvent(event);
}

void WebViewportItem::commitZoom()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_owner->commitZoom(m_zoomScale);
}
