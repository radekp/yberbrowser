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

#ifndef TileContainerWidget_h_
#define TileContainerWidget_h_

#include <QGraphicsWidget>
#include "TileItem.h"

class QGraphicsSceneMouseEvent;
class QParallelAnimationGroup;
class QGraphicsSimpleTextItem;

class TileBaseWidget : public QGraphicsWidget {
    Q_OBJECT
public:
    virtual ~TileBaseWidget();

    virtual void addTile(TileItem& newItem);
    virtual void removeTile(TileItem& removed);
    virtual void removeAll();
    virtual void layoutTiles() = 0;

    void setEditMode(bool on);
    bool editMode() const { return m_editMode; }

Q_SIGNALS:
    void closeWidget();

protected:
    TileBaseWidget(const QString& title, QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);

    QSize doLayoutTiles(const QRectF& rect, int hTileNum, int vTileNum, int marginX, int marginY, bool fixed = false);

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    void addMoveAnimation(TileItem& item, int delay, const QPointF& oldPos, const QPointF& newPos);

protected:
    TileList m_tileList;

private:
    QGraphicsSimpleTextItem* m_titleItem;
    QParallelAnimationGroup* m_slideAnimationGroup;
    QString m_title;
    bool m_editMode;
};

// subclasses
// #########
class TabWidget : public TileBaseWidget {
    Q_OBJECT
public:
    TabWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);
    void layoutTiles();

    void removeTile(TileItem& removed);
};

class HistoryWidget : public TileBaseWidget {
    Q_OBJECT
public:
    HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);
    
    void removeTile(TileItem& removed);
    void layoutTiles();
};

class BookmarkWidget : public TileBaseWidget {
    Q_OBJECT
public:
    BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);

    void removeTile(TileItem& removed);
    void layoutTiles();
};

class PopupWidget : public TileBaseWidget {
    Q_OBJECT
public:
    PopupWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);

    void removeTile(TileItem& removed);
    void layoutTiles();
};

#endif
