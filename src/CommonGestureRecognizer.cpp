/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <QtGui>

#include "CommonGestureRecognizer.h"
#include "EventHelpers.h"

// time between mouse release that was part of pan and
// double tap that can happen
static const int s_doubleClickFilterDurationMS = 300;

static const int s_minTimeHoldForClick = 100;

/* TODO

    - This should go away

    - At max, this should synthetize TouchEvents

    - The event coords are not welldefined, since the events might
      sometimes come from the container (WebViewportItem), sometimes
      from the childs (Web View)

    - This should do mouse click filtering, as it is very unreliable
      on device.
 */

/*!
  \class CommonGestureRecognizer class that filters unreliable mouse input and
  forwards them to the consumer.

  Called from \QGraphicsItem::sceneEventFilter
*/
CommonGestureRecognizer::CommonGestureRecognizer(CommonGestureConsumer* consumer)
    : m_consumer(consumer)
    , m_delayedPressEvent(0)
    , m_delayedReleaseEvent(0)
    , m_pressDelay(300)

{
    reset();
}

CommonGestureRecognizer::~CommonGestureRecognizer()
{
    clearDelayedPress();
}

bool CommonGestureRecognizer::filterMouseEvent(QGraphicsSceneMouseEvent *event)
{
    // these events will be sent by this class, don't filter these
    if (m_delayedPressEvent == event || m_delayedReleaseEvent == event)
        return false;

#if 0
    // mapping is not needed at the moment, but
    // this is here because it's so valuable and big piece of
    // elegant code
    QGraphicsSceneMouseEvent mappedEvent(event->type());
    mapEventCommonProperties(this, event, mappedEvent);

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
#endif

    bool accepted = false;

    switch(event->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
        clearDelayedPress();
        accepted = mouseDoubleClickEvent(event);
        break;
    case QEvent::GraphicsSceneMouseMove:
        accepted = mouseMoveEvent(event);
        break;
    case QEvent::GraphicsSceneMousePress:
        accepted = mousePressEvent(event);
        break;

    case QEvent::GraphicsSceneMouseRelease:
        accepted = mouseReleaseEvent(event);
        break;
    default:
        break;
    }

    return accepted;
}

void CommonGestureRecognizer::capturePressOrRelease(QGraphicsSceneMouseEvent *event, bool wasRelease)
{
    if (m_pressDelay <= 0)
        return;

    QGraphicsSceneMouseEvent* delayedEvent = new QGraphicsSceneMouseEvent(event->type());
    copyMouseEvent(event, delayedEvent);
    delayedEvent->setAccepted(false);

    if (wasRelease) {
        // let the client do link finding on release event
        QPointF pos = m_delayedPressEvent->scenePos();
        m_consumer->adjustClickPosition(pos);
        // FIXME FIXME
        m_delayedPressEvent->setScenePos(pos);
        m_delayedReleaseEvent = delayedEvent;
        // mouse press is more reliable than mouse release
        // we must send release with same coords as press
        copyMouseEventPositions(m_delayedPressEvent, m_delayedReleaseEvent);
        m_delayedPressTimer.start(m_pressDelay, this);
    } else {
        m_delayedPressEvent = delayedEvent;
        m_delayedPressMoment.start();
    }
}

void CommonGestureRecognizer::clearDelayedPress()
{
    if (m_delayedPressEvent) {
        m_delayedPressTimer.stop();
        delete m_delayedPressEvent;
        m_delayedPressEvent = 0;
        delete m_delayedReleaseEvent;
        m_delayedReleaseEvent = 0;
    }
}

void CommonGestureRecognizer::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_delayedPressTimer.timerId()) {
        m_delayedPressTimer.stop();
        m_consumer->mousePressEventFromChild(m_delayedPressEvent, true);
        m_consumer->mouseReleaseEventFromChild(m_delayedReleaseEvent);
        clearDelayedPress();
    }
}

bool CommonGestureRecognizer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos() << " filter: " <<m_doubleClickFilter.elapsed();
#endif

    if (m_doubleClickFilter.elapsed() > s_doubleClickFilterDurationMS)
        m_consumer->mouseDoubleClickEventFromChild(event);
    else
        mousePressEvent(event);
    return true;
}


bool CommonGestureRecognizer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos();
#endif

    m_consumer->mousePressEventFromChild(event, false);

    // long tap causes left button click, don't send it further
    if (event->button() != Qt::LeftButton)
        return true;

    if (!m_delayedPressEvent)
        capturePressOrRelease(event);

    return true;
}

bool CommonGestureRecognizer::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos();
#endif

    if (event->button() != Qt::LeftButton)
        return true;

    if (m_delayedReleaseEvent) {
        // sometimes double click is lost if small mouse move occurs
        // inbetween
        QGraphicsSceneMouseEvent dblClickEvent(QEvent::GraphicsSceneMouseDoubleClick);
        copyMouseEvent(m_delayedPressEvent, &dblClickEvent);
        mouseDoubleClickEvent(&dblClickEvent);
    } else if (m_delayedPressEvent && !m_delayedReleaseEvent) {
        if (m_delayedPressMoment.elapsed() > s_minTimeHoldForClick)
            capturePressOrRelease(event, true);
        else
            clearDelayedPress();
    }
    return true;
}

bool CommonGestureRecognizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos();
#endif
    Q_UNUSED(event);
    // filter anyway since we don't want excess link hovers when panning.
    return true;
}

void CommonGestureRecognizer::reset()
{
    clearDelayedPress();
}


