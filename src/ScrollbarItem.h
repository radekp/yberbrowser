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

#ifndef ScrollbarItem_h_
#define ScrollbarItem_h_

#include <QObject>
#include <QRectF>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QPropertyAnimation>

class QGraphicsWidget;


class ScrollbarItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ScrollbarItem(Qt::Orientation orientation, QGraphicsItem* parent = 0);
    ~ScrollbarItem();

    void setMargins(int top, int bottom);

    void updateVisibilityAndFading(bool shouldFadeOut);

    void contentPositionUpdated(qreal pos, qreal contentLength, const QSizeF& viewSize, bool shouldFadeOut);

protected Q_SLOTS:
    void startFadeOut();
    void fadingFinished();

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void startFading(bool in);

private:
    Qt::Orientation m_orientation;
    QPropertyAnimation m_fadeAnim;
    QTimer m_fadeOutTimeout;
    int m_topMargin;
    int m_bottomMargin;
};

#endif
