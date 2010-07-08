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

#include "PannableViewport.h"
#include "ScrollbarItem.h"
#include "EventHelpers.h"

#include <QPointF>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

namespace {
const unsigned s_scrollsPerSecond = 60;
const qreal s_axisLockThreshold = .7;
}

/*!
  \class PannableViewport \QGraphicsItem that acts as a viewport for its children.

  Responsibilities:
  * Implements panning interaction

  Corresponds to MPannableViewport in MeegoTouch, but is implemented via
  \QGraphicsItem::setFiltersChildEvents

  FIXME: currently bugs when content is resized. This will not
  cause scrollbars to appear. To support this, we need a generic
  way to detect resizes. This can be done only by creating custom
  layout
*/

PannableViewport::PannableViewport(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_pannedWidget(0)
    , m_vScrollbar(new ScrollbarItem(Qt::Vertical, this))
    , m_hScrollbar(new ScrollbarItem(Qt::Horizontal, this))
{
//    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFiltersChildEvents(true);

    setScrollsPerSecond(s_scrollsPerSecond);
    setOvershootPolicy(YberHack_Qt::QAbstractKineticScroller::OvershootWhenScrollable);
    setAxisLockThreshold(s_axisLockThreshold);
}

PannableViewport::~PannableViewport()
{
}

void PannableViewport::setPosition(const QPointF& pos)
{
    QRectF oldGeometry = m_pannedWidget->geometry();

    m_pannedWidget->setPos(pos);

    QRectF newGeometry = m_pannedWidget->geometry();

    if (oldGeometry != newGeometry) {
        updateScrollbars();
        emit positionChanged(newGeometry);
    }
}

QPointF PannableViewport::position() const
{
    return m_pannedWidget->pos() - m_overShootDelta;
}

void PannableViewport::setRange(const QRectF& )
{
}

void PannableViewport::setWidget(QGraphicsWidget* view)
{
    if (view == m_pannedWidget)
        return;

    if (m_pannedWidget) {
        m_pannedWidget->setParentItem(0);
        delete m_pannedWidget;
    }

    m_pannedWidget = view;
    m_pannedWidget->setParentItem(this);
    m_pannedWidget->stackBefore(m_vScrollbar);
}

QPoint PannableViewport::maximumScrollPosition() const
{
    QSizeF contentsSize = m_pannedWidget->size();
    QSizeF sz = size();
    QSize maxSize = (contentsSize - sz).toSize();

    return QPoint(qMax(0, maxSize.width()), qMax(0, maxSize.height()));
}

QSize PannableViewport::viewportSize() const
{
    return size().toSize();
}

QPoint PannableViewport::scrollPosition() const
{
    return (-position()).toPoint();
}

void PannableViewport::updateScrollbars()
{
    // FIXME: find out how to update marging based on the attached widget when the widget boundingrect is changed
    m_vScrollbar->setMargins(5, -1);

    QPointF contentPos = position();
    QSizeF contentSize = m_pannedWidget->size();

    bool shouldFadeOut = !(state() == YberHack_Qt::QAbstractKineticScroller::MousePressed || state() == YberHack_Qt::QAbstractKineticScroller::Pushing);

    m_hScrollbar->contentPositionUpdated(contentPos.x(), contentSize.width(), size(), shouldFadeOut);
    m_vScrollbar->contentPositionUpdated(contentPos.y(), contentSize.height(), size(), shouldFadeOut);
}

void PannableViewport::setScrollPosition(const QPoint &pos, const QPoint &overShootDelta)
{
    m_overShootDelta = overShootDelta;
    setPosition(-pos);
}

void PannableViewport::stateChanged(YberHack_Qt::QAbstractKineticScroller::State oldState, YberHack_Qt::QAbstractKineticScroller::State newState)
{
    YberHack_Qt::QAbstractKineticScroller::stateChanged(oldState, newState);
    updateScrollbars();

    if (newState == YberHack_Qt::QAbstractKineticScroller::Inactive)
        emit panningStopped();
}

bool PannableViewport::sceneEvent(QEvent* e)
{
    bool doFilter = false;

    switch (e->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        doFilter = handleMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;
    default:
        break;
    }
    return doFilter;
}

bool PannableViewport::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    if (!isVisible())
        return QGraphicsItem::sceneEventFilter(i, e);

    bool doFilter = false;

    switch (e->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        doFilter = handleMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;

    default:
        break;
    }

    return doFilter ? true : QGraphicsItem::sceneEventFilter(i, e);
}
