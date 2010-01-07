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

static const int s_defaultZoomLevel = 5;
static const int s_zoomValues[] = {30, 50, 67, 80, 90, 100, 110, 120, 133, 150, 170, 200, 240, 300};
static const int s_numOfZoomLevels = int(sizeof(s_zoomValues) / sizeof(int)) - 1;

WebViewportItem::WebViewportItem(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_webView(0)
    , m_zoomAnim(this, "zoomScale")
    , m_panAnim(this)
    , m_zoomCommitTimer(this)
{
#if !defined(ENABLE_PAINT_DEBUG)
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFiltersChildEvents(true);
    m_panAnim.setTimeLine(new QTimeLine(s_zoomAnimDurationMS));
    connect(m_panAnim.timeLine(), SIGNAL(stateChanged(QTimeLine::State)), this, SLOT(panAnimStateChanged(QTimeLine::State)));

    m_zoomAnim.setDuration(s_zoomAnimDurationMS);
    connect(&m_zoomCommitTimer, SIGNAL(timeout()), this, SLOT(commitZoom()));
    m_zoomCommitTimer.setSingleShot(true);

    resetState(true);
}

void WebViewportItem::panAnimStateChanged(QTimeLine::State newState)
{
    switch(newState) {
    case QTimeLine::Running:
        m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
        break;
    case QTimeLine::NotRunning: {
        m_panAnim.timeLine()->stop();
        qreal s = m_panAnim.horizontalScaleAt(1);
        QPointF p = m_panAnim.posAt(1);

        // we have to clear the anim transforms
        m_panAnim.clear();
        // calling just QGraphicsItemAnimation::clear(),setPos(0) does
        // not work it leaves the scaling there
        m_panAnim.setScaleAt(0, 1., 1.);
        m_panAnim.setPosAt(0, QPointF(0,0));
        m_panAnim.setStep(0);

        m_webView->setPos(p);
        m_webView->setScale(s);
        m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
        break;
    }
    case QTimeLine::Paused:
    default:
        break;
    }
}

qreal WebViewportItem::zoomScaleForZoomLevel() const
{
    return s_zoomValues[m_zoomLevel] / 100.;
}

void WebViewportItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        QGraphicsWidget::mousePressEvent(event);
        return;
    }
    m_isDragging = true;
    m_dragStartPos = event->pos();
    event->accept();
}

void WebViewportItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        QGraphicsWidget::mouseReleaseEvent(event);
        return;
    }

    m_isDragging = false;
    event->accept();
}

void WebViewportItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isDragging) {
        QPointF d = event->pos() - m_dragStartPos;
        m_webView->moveBy(d.x(), d.y());
        m_dragStartPos = event->pos();
    }
}

void WebViewportItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    //setZoomLevel(zoomLevel() + 1);
    event->accept();

    QPointF p = mapToItem(m_webView, event->pos());

    QWebHitTestResult r = m_webView->page()->mainFrame()->hitTestContent(p.toPoint());

    QSize cs = m_webView->page()->mainFrame()->contentsSize();

    QSize ts = r.boundingRect().size();
    qreal targetScale = static_cast<qreal>(cs.width()) / ts.width();

    QPointF targetPoint =  (-r.boundingRect().topLeft()) * targetScale;

    qreal curScale = m_webView->scale();

    if (targetScale == zoomScale()) {
        targetScale = static_cast<qreal>(size().width()) / cs.width();
        targetPoint = QPointF(0, (m_webView->pos().y()/curScale)*targetScale);
    }

    QPointF curPos = m_webView->pos();

    m_webView->setPos(QPointF(0,0));
    m_webView->setScale(1.);
    m_panAnim.setPosAt(0, curPos);
    m_panAnim.setPosAt(1, targetPoint);
    m_panAnim.setScaleAt(0, curScale, curScale);
    m_panAnim.setScaleAt(1, targetScale, targetScale);
    m_panAnim.timeLine()->start();
}

qreal WebViewportItem::zoomScale()
{
    if (!m_webView)
        return 1.;

    return m_webView->scale();
}

void WebViewportItem::setZoomScale(qreal value)
{
    if (value != zoomScale()) {
        m_zoomScale = value;
        m_webView->setScale(value);
    }

    m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
    m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
}

void WebViewportItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    int adv = event->delta() / (15*8);
    setZoomLevel(zoomLevel() + adv);
    event->accept();
}

int WebViewportItem::zoomLevel() const
{
    return m_zoomLevel;
}

void WebViewportItem::setZoomLevel(int level)
{
    m_zoomLevel = qBound(0, level, s_numOfZoomLevels);
    animateZoomScaleTo(zoomScaleForZoomLevel());
}

void WebViewportItem::animateZoomScaleTo(qreal targetZoomScale)
{
    m_zoomAnim.stop();
    m_zoomAnim.setStartValue(zoomScale());
    m_zoomAnim.setEndValue(targetZoomScale);
    m_zoomAnim.start();
}

void WebViewportItem::commitZoom()
{
    qreal s = m_webView->scale();
    m_webView->setTileCacheZoomFactorX(s);
    m_webView->setTileCacheZoomFactorY(s);
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
    m_webView->setAttribute(Qt::WA_OpaquePaintEvent, true);
    m_panAnim.setItem(m_webView);
}

void WebViewportItem::resetState(bool resetZoom)
{
    if (m_webView)
        m_webView->setPos(QPointF(0, 0));

    if (resetZoom) {
        m_zoomLevel = s_defaultZoomLevel;
    }

    m_zoomScale = zoomScaleForZoomLevel();

    if (m_webView)
        m_webView->setScale(m_zoomScale);

    m_isDragging = false;
    m_dragStartPos = QPoint();
    m_zoomAnim.stop();
    m_panAnim.timeLine()->stop();
}

#if defined(ENABLE_PAINT_DEBUG)
void WebViewportItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    static int n = 0;
    ++n;
    qDebug() << "WebViewportItem::paint" << option->exposedRect.toRect() << boundingRect() << (n % 2 ? "Qt::green" : "Qt::darkGreen");

    painter->save();
    painter->setPen(Qt::green);
    painter->setBrush(Qt::green);
    painter->fillRect(option->exposedRect.toRect(), n % 2 ? Qt::green : Qt::darkGreen );
    painter->restore();
    QGraphicsWidget::paint(painter, option, widget);
}
#endif

/*!
  Copies common properties of mouse/wheel event to new event. Maps the coordinates from the event
  to the coordinate space of the receiving item

  This is very errorprone and should be replaced.
  \internal
 */
template<typename EventType>
bool mapEventCommonPropertiesIfApplicable(QGraphicsItem* parent, const EventType* event, EventType& mappedEvent)
{
    // lifted form qmlgraphicsflickable.cpp, LGPL Nokia
    QRectF parentRect = parent->mapToScene(parent->boundingRect()).boundingRect();
    if (!parentRect.contains(event->scenePos().toPoint()))
        return false;

    mappedEvent.setAccepted(false);
    mappedEvent.setPos(parent->mapFromScene(event->scenePos()));
    mappedEvent.setScenePos(event->scenePos());
    mappedEvent.setScreenPos(event->screenPos());
    mappedEvent.setModifiers(event->modifiers());
    mappedEvent.setButtons(event->buttons());

    return true;
}

bool WebViewportItem::sendMouseEventFromChild(QGraphicsSceneMouseEvent *event)
{
    QGraphicsSceneMouseEvent mappedEvent(event->type());
    if (!mapEventCommonPropertiesIfApplicable(this, event, mappedEvent))
        return false;

    for (int i = 0x1; i <= 0x10; i <<= 1) {
        if (event->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            mappedEvent.setButtonDownPos(button, mapFromScene(event->buttonDownPos(button)));
            mappedEvent.setButtonDownScenePos(button, event->buttonDownScenePos(button));
            mappedEvent.setButtonDownScreenPos(button, event->buttonDownScreenPos(button));
        }
    }

    mappedEvent.setLastPos(mapFromScene(event->lastScenePos()));
    mappedEvent.setLastScenePos(event->lastScenePos());
    mappedEvent.setLastScreenPos(event->lastScreenPos());
    mappedEvent.setButton(event->button());


    switch(mappedEvent.type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
        mouseDoubleClickEvent(&mappedEvent);
        break;
    case QEvent::GraphicsSceneMouseMove:
        mouseMoveEvent(&mappedEvent);
        break;
    case QEvent::GraphicsSceneMousePress:
        mousePressEvent(&mappedEvent);
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouseReleaseEvent(&mappedEvent);
        break;
    default:
        break;
    }
    return mappedEvent.isAccepted();
}

bool WebViewportItem::sendWheelEventFromChild(QGraphicsSceneWheelEvent *event)
{
    QGraphicsSceneWheelEvent mappedEvent(event->type());
    if (!mapEventCommonPropertiesIfApplicable(this, event, mappedEvent))
        return false;

    mappedEvent.setDelta(event->delta());
    mappedEvent.setOrientation(event->orientation());

    wheelEvent(&mappedEvent);
    return mappedEvent.isAccepted();
}

bool WebViewportItem::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    // lifted form qmlgraphicsflickable.cpp, LGPL Nokia

    if (!isVisible())
        return QGraphicsItem::sceneEventFilter(i, e);


    switch (e->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        return sendMouseEventFromChild(static_cast<QGraphicsSceneMouseEvent *>(e));

    case QEvent::GraphicsSceneWheel:
        return sendWheelEventFromChild(static_cast<QGraphicsSceneWheelEvent*>(e));
        break;

    default:
        return QGraphicsItem::sceneEventFilter(i, e);
    }

    if (e->isAccepted())
        return true;

    return QGraphicsItem::sceneEventFilter(i, e);
}
