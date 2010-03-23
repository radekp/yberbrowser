/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include <QDebug>
#include <QPointF>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include "PannableViewport.h"
#include "ScrollbarItem.h"
#include "EventHelpers.h"

static const unsigned s_scrollsPerSecond = 60;
static const qreal s_axisLockThreshold = .4;


PannableViewport::PannableViewport(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_pannedWidget(0)
    , m_vScrollbar(new ScrollbarItem(Qt::Vertical, this))
    , m_hScrollbar(new ScrollbarItem(Qt::Horizontal, this))
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFiltersChildEvents(true);

    setScrollsPerSecond(s_scrollsPerSecond);
    setOvershootPolicy(YberHack_Qt::QAbstractKineticScroller::OvershootAlwaysOn);
    setAxisLockThreshold(s_axisLockThreshold);
}

PannableViewport::~PannableViewport()
{
}

void PannableViewport::setPanPos(const QPointF& pos)
{
    setWebViewPos(pos);
}

QPointF PannableViewport::panPos() const
{
    return m_pannedWidget->pos();
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
    return (-(panPos() - m_overShootDelta)).toPoint();
}

void PannableViewport::updateScrollbars()

{
    if (!m_vScrollbar || !m_hScrollbar)
        return;
    QPointF contentPos = m_pannedWidget->pos();
    QSizeF contentSize = m_pannedWidget->size();

    QSizeF viewSize = size();

    bool shouldFadeOut = !(state() == YberHack_Qt::QAbstractKineticScroller::MousePressed || state() == YberHack_Qt::QAbstractKineticScroller::Pushing);

    m_hScrollbar->contentPositionUpdated(contentPos.x(), contentSize.width(), viewSize, shouldFadeOut);
    m_vScrollbar->contentPositionUpdated(contentPos.y(), contentSize.height(), viewSize, shouldFadeOut);
}

void PannableViewport::setScrollPosition(const QPoint &pos, const QPoint &overShootDelta)
{
    m_overShootDelta = overShootDelta;
    setWebViewPos(-(pos - overShootDelta));
}

void PannableViewport::stateChanged(YberHack_Qt::QAbstractKineticScroller::State oldState)
{
    YberHack_Qt::QAbstractKineticScroller::stateChanged(oldState);
    updateScrollbars();
}

bool PannableViewport::canStartScrollingAt(const QPoint &globalPos) const
{
    return YberHack_Qt::QAbstractKineticScroller::canStartScrollingAt(globalPos);
}

void PannableViewport::setWebViewPos(const QPointF& point)
{
    m_pannedWidget->setPos(point);
    updateScrollbars();
}

bool PannableViewport::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    qDebug() << __PRETTY_FUNCTION__;

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



QPointF PannableViewport::clipPointToViewport(const QPointF& p) const
{
    QSizeF contentsSize = size();
    QSizeF sz = size();

    qreal minX = -qMax(contentsSize.width() - sz.width(), static_cast<qreal>(0.));
    qreal minY = -qMax(contentsSize.height() - sz.height(), static_cast<qreal>(0.));

    return QPointF(qBound(minX, p.x(), static_cast<qreal>(0.)),
                   qBound(minY, p.y(), static_cast<qreal>(0.)));
}




