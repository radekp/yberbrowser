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

static const float s_zoomScaleWheelStep = .2;
static const qreal s_minZoomScale = .01; // arbitrary
static const qreal s_maxZoomScale = 10.; // arbitrary


WebViewportItem::WebViewportItem(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_webView(0)
    , m_zoomAnim(this)
    , m_zoomCommitTimer(this)
{
#if !defined(ENABLE_PAINT_DEBUG)
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFiltersChildEvents(true);

    m_zoomAnim.setTimeLine(new QTimeLine(s_zoomAnimDurationMS));
    connect(m_zoomAnim.timeLine(), SIGNAL(stateChanged(QTimeLine::State)), this, SLOT(panAnimStateChanged(QTimeLine::State)));

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
        transferAnimStateToView();
        m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
        break;
    }
    case QTimeLine::Paused:
        // ### what to do?
    default:
        break;
    }
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

    startZoomAnimTo(targetPoint, targetScale);
}

void WebViewportItem::startZoomAnimTo(const QPointF& targetPoint, qreal targetScale)
{
    m_zoomAnim.timeLine()->stop();
    qreal step = m_zoomAnim.timeLine()->currentValue();

    qreal curScale = m_zoomAnim.horizontalScaleAt(step) * m_webView->scale();
    QPointF curPos = m_zoomAnim.posAt(step) + m_webView->pos();

    m_webView->setPos(QPointF(0,0));
    m_webView->setScale(1.);

    m_zoomAnim.setPosAt(0, curPos);
    m_zoomAnim.setPosAt(1, targetPoint);
    m_zoomAnim.setScaleAt(0, curScale, curScale);
    m_zoomAnim.setScaleAt(1, targetScale, targetScale);
    m_zoomAnim.setStep(0);
    m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
    m_zoomAnim.timeLine()->start();
}

void WebViewportItem::transferAnimStateToView()
{
    Q_ASSERT(m_zoomAnim.timeLine()->state == QTimeLine::NotRunning);

    qreal step = m_zoomAnim.timeLine()->currentValue();

    qreal s = m_zoomAnim.horizontalScaleAt(step);
    QPointF p = m_zoomAnim.posAt(step);

    resetZoomAnim();

    m_webView->setPos(p);
    m_webView->setScale(s);
}


qreal WebViewportItem::zoomScale()
{
    if (!m_webView)
        return 1.;

    return m_zoomAnim.horizontalScaleAt(1) * m_webView->scale();
}

void WebViewportItem::setZoomScale(qreal value, bool commitInstantly)
{
    value = qBound(s_minZoomScale, value, s_maxZoomScale);

    if (value != zoomScale()) {
        m_webView->setScale(value);
    }

    if (commitInstantly) {
        commitZoom();
    } else {
        m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
        m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
    }
}

void WebViewportItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    int adv = event->delta() / (15*8);
    animateZoomScaleTo(zoomScale() + adv * s_zoomScaleWheelStep);
    event->accept();
}

void WebViewportItem::animateZoomScaleTo(qreal targetZoomScale)
{
    targetZoomScale = qBound(s_minZoomScale, targetZoomScale, s_maxZoomScale);
    startZoomAnimTo(m_webView->pos(), targetZoomScale);
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
    m_zoomAnim.setItem(m_webView);
}

void WebViewportItem::resetState(bool resetZoom)
{
    if (m_webView)
        m_webView->setPos(QPointF(0, 0));

    m_zoomAnim.setPosAt(0, QPointF(0,0));
    m_zoomAnim.setPosAt(1, QPointF(0,0));

    if (resetZoom) {
        if (m_webView) {
            m_webView->setScale(1.);
            m_webView->setTileCacheZoomFactorX(1.);
            m_webView->setTileCacheZoomFactorY(1.);
            m_webView->setTileCacheState(QWebFrame::TileCacheNormal);
        }
        m_zoomAnim.timeLine()->stop();
        resetZoomAnim();
    }

    m_isDragging = false;
    m_dragStartPos = QPoint();
}

void WebViewportItem::resetZoomAnim()
{
    // we have to clear the anim transforms
    m_zoomAnim.clear();
    // calling just QGraphicsItemAnimation::clear(),setPos(0) does
    // not work. it leaves the scaling there
    m_zoomAnim.setScaleAt(0, 1., 1.);
    m_zoomAnim.setPosAt(0, QPointF(0,0));
    m_zoomAnim.setStep(0);
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

    default:
        break;
    }

    return QGraphicsItem::sceneEventFilter(i, e);
}

QGraphicsWebView* WebViewportItem::webView()
{
    return m_webView;
}
