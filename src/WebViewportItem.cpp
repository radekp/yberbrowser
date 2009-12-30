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

static const int s_zoomAnimDurationMS = 300;
static const int s_zoomCommitTimerDurationMS = 500;

static const int s_zoomValues[] = {30, 50, 67, 80, 90, 100, 110, 120, 133, 150, 170, 200, 240, 300};
static const int s_numOfZoomLevels = int(sizeof(s_zoomValues) / sizeof(int)) - 1;

WebViewportItem::WebViewportItem(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_webView(0)
    , m_isDragging(false)
    , m_zoomLevel(5)
    , m_zoomAnim(this, "zoomScale")
    , m_zoomCommitTimer(this)
{
    m_zoomScale = zoomScaleForZoomLevel();

    m_zoomAnim.setDuration(s_zoomAnimDurationMS);
    connect(&m_zoomCommitTimer, SIGNAL(timeout()), this, SLOT(commitZoom()));
    m_zoomCommitTimer.setSingleShot(true);

}

qreal WebViewportItem::zoomScaleForZoomLevel() const
{
    return s_zoomValues[m_zoomLevel] / 100.;
}

void WebViewportItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_isDragging = true;
    m_dragStartPos = event->pos();
}

void WebViewportItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    m_isDragging = false;
    m_pos += event->pos() - m_dragStartPos;
}

void WebViewportItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isDragging) {
        m_webView->setPos(m_pos + (event->pos() - m_dragStartPos));
    }
}

qreal WebViewportItem::zoomScale()
{
    return m_zoomScale;
}

void WebViewportItem::setZoomScale(qreal value)
{
    if (value != m_zoomScale) {
        m_zoomScale = value;
        m_webView->setScale(m_zoomScale);
    }

    qDebug() << "zooming: " << m_zoomScale;
    m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
    m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
}

void WebViewportItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    int adv = event->delta() / (15*8);
    m_zoomLevel = qBound(0, m_zoomLevel + adv, s_numOfZoomLevels);

    m_zoomAnim.stop();
    m_zoomAnim.setStartValue(m_zoomScale);
    m_zoomAnim.setEndValue(zoomScaleForZoomLevel());
    m_zoomAnim.start();
}

void WebViewportItem::commitZoom()
{
    m_webView->setTileCacheZoomFactorX(m_zoomScale);
    m_webView->setTileCacheZoomFactorY(m_zoomScale);
    m_webView->setTileCacheState(QWebFrame::TileCacheNormal);
}

void WebViewportItem::setWebView(QGraphicsWebView* view)
{
    if (m_webView) {
        m_webView->setParentItem(0);
        delete m_webView;
    }

    m_webView = view;
    m_webView->setParentItem(this);
}

