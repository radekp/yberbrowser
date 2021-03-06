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

#ifndef PannableViewport_h
#define PannableViewport_h

#include "yberconfig.h"

#if USE_MEEGOTOUCH
#include <MPannableViewport>
class PannableViewport : public MPannableViewport
{
public:
    explicit PannableViewport(QGraphicsItem* parent = 0)
        : MPannableViewport(parent)
        {
        }
};

#else

#include <QGraphicsWidget>
#include <QPropertyAnimation>

#include "3rdparty/qabstractkineticscroller.h"

class ScrollbarItem;
class QGraphicsItem;

class PannableViewport : public QGraphicsWidget, private YberHack_Qt::QAbstractKineticScroller
{
    Q_OBJECT
    Q_DISABLE_COPY(PannableViewport);
    Q_PROPERTY(QPointF position READ position WRITE setPosition)

public:
    explicit PannableViewport(QGraphicsItem* parent = 0);
    ~PannableViewport();

    void setPosition(const QPointF& pos);
    QPointF position() const;

    void setRange(const QRectF&);
    void setAutoRange(bool) { }
    void setPanDirection(Qt::Orientations) {}
    

    void setWidget(QGraphicsWidget*);
    QGraphicsWidget* widget() const { return m_pannedWidget; }

Q_SIGNALS:
    void panningStopped();
    void positionChanged(const QRectF&);

protected:
    bool sceneEvent(QEvent* e);
    bool sceneEventFilter(QGraphicsItem *i, QEvent *e);

private:
    void updateScrollbars();

    QSize viewportSize() const;
    QPoint maximumScrollPosition() const;
    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint &pos, const QPoint &overShootDelta);
    void stateChanged(YberHack_Qt::QAbstractKineticScroller::State oldState, YberHack_Qt::QAbstractKineticScroller::State newState);

private:
    QGraphicsWidget* m_pannedWidget;
    ScrollbarItem* m_vScrollbar;
    ScrollbarItem* m_hScrollbar;

    QPointF m_overShootDelta;
};
#endif

#endif
