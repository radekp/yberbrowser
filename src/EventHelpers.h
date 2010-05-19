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

#ifndef EventHelpers_h
#define EventHelpers_h

#include <QEvent>
#include <QGraphicsItem>

class QGraphicsSceneMouseEvent;

void copyMouseEvent(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to);

void copyMouseEventPositions(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to);

/*!
  Copies common properties of mouse/wheel event to new event. Maps the coordinates from the event
  to the coordinate space of the receiving item

  This is very errorprone and should be replaced.
  \internal
 */
template<typename EventType>
void mapEventCommonProperties(QGraphicsItem* parent, const EventType* event, EventType& mappedEvent)
{
    // lifted form qmlgraphicsflickable.cpp, LGPL Nokia

    QRectF parentRect = parent->mapToScene(parent->boundingRect()).boundingRect();

    mappedEvent.setAccepted(false);
    mappedEvent.setPos(parent->mapFromScene(event->scenePos()));
    mappedEvent.setScenePos(event->scenePos());
    mappedEvent.setScreenPos(event->screenPos());
    mappedEvent.setModifiers(event->modifiers());
    mappedEvent.setButtons(event->buttons());
}



#endif
