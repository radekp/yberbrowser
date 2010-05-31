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

#include "EventHelpers.h"

#include <QGraphicsSceneMouseEvent>

void copyMouseEvent(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to)
{
    for (int i = 0x1; i <= 0x10; i <<= 1) {
        if (from->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            to->setButtonDownPos(button, from->buttonDownPos(button));
            to->setButtonDownScenePos(button, from->buttonDownScenePos(button));
            to->setButtonDownScreenPos(button, from->buttonDownScreenPos(button));
        }
    }
    to->setAccepted(from->isAccepted());
    to->setButtons(from->buttons());
    to->setButton(from->button());
    to->setModifiers(from->modifiers());
    to->setWidget(from->widget());

    copyMouseEventPositions(from, to);
}

void copyMouseEventPositions(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to)
{
    to->setPos(from->pos());
    to->setScenePos(from->scenePos());
    to->setScreenPos(from->screenPos());
    to->setLastPos(from->lastPos());
    to->setLastScenePos(from->lastScenePos());
    to->setLastScreenPos(from->lastScreenPos());
}

