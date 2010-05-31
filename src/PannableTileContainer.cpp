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

#include "PannableTileContainer.h"
#include "TileSelectionViewBase.h" // FIXME temp

#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QGraphicsScene>
#include <QCoreApplication>

PannableTileContainer::PannableTileContainer(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : PannableViewport(parent, wFlags)
{
}

PannableTileContainer::~PannableTileContainer()
{
}

bool PannableTileContainer::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    bool doFilter = PannableViewport::sceneEventFilter(i, e);

    if (!isVisible() || doFilter)
        return doFilter;

    switch (e->type()) {
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick:
        return (static_cast<TileSelectionViewBase *>(parentWidget()))->filterMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;
    default:
        break;
    }
    return false;
}

