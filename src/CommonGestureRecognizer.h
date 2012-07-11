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

#ifndef CommonGestureRecognizer_h_
#define CommonGestureRecognizer_h_

#include <QBasicTimer>
#include <QObject>
#include <QPointF>
#include <QTime>

class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class QPointF;

class CommonGestureConsumer
{

public:
    virtual void mousePressEventFromChild(QGraphicsSceneMouseEvent*, bool filtered) = 0;
    virtual void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent*) = 0;
    virtual void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent*) = 0;
    virtual void adjustClickPosition(QPointF&) = 0;
};

class CommonGestureRecognizer : public QObject
{
    Q_OBJECT
public:
    CommonGestureRecognizer(CommonGestureConsumer*);
    ~CommonGestureRecognizer();

    bool filterMouseEvent(QGraphicsSceneMouseEvent *);

    void reset();
    void clearDelayedPress();

protected:
    void timerEvent(QTimerEvent *event);

private:
    void capturePressOrRelease(QGraphicsSceneMouseEvent *event, bool wasRelease = false);
    void sendDelayedPress();

    bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    bool mousePressEvent(QGraphicsSceneMouseEvent* event);
    bool mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    bool mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    CommonGestureConsumer* m_consumer;

    QGraphicsSceneMouseEvent* m_delayedPressEvent;
    QGraphicsSceneMouseEvent* m_delayedReleaseEvent;
    QTime m_delayedPressMoment;
    QTime m_doubleClickFilter;
    QBasicTimer m_delayedPressTimer;
    int m_pressDelay;
};

#endif
