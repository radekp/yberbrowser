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

#ifndef TileSelectionViewBase_h_
#define TileSelectionViewBase_h_

#include <QGraphicsWidget>

#include "ApplicationWindow.h"
class QGraphicsPixmapItem;
class TileItem;
class QGraphicsSceneMouseEvent;

class TileSelectionViewBase : public QGraphicsWidget {
    Q_OBJECT
public:
    enum ViewType {
        Home,
        UrlPopup
    };

    virtual ~TileSelectionViewBase();

    void appear();
    void disappear();

    void setGeometry(const QRectF&);
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    ViewType viewtype() const { return m_type; }

    void updateBackground(QGraphicsPixmapItem* bckg);
    void updateContent();

    // FIXME temp hack until event handling is fixed
    virtual bool filterMouseEvent(QGraphicsSceneMouseEvent*) { return false; }

Q_SIGNALS:
    void viewDismissed();

public Q_SLOTS:
    virtual void tileItemActivated(TileItem*) {}
    virtual void tileItemClosed(TileItem*) {}
    virtual void tileItemEditingMode(TileItem*) {}

protected:
    TileSelectionViewBase(ViewType type, QGraphicsPixmapItem* bckg, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);

    virtual void resetContainerSize() = 0;
    virtual void createViewItems() = 0;
    virtual void destroyViewItems() = 0;
    virtual void connectItem(TileItem&);

protected Q_SLOTS:
    void closeView();
    void closeViewSoon();

private:
    QGraphicsPixmapItem* m_bckg;
    ViewType m_type;
};
#endif
