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

#ifndef TileItem_h_
#define TileItem_h_

#include <QObject>
#include <QRectF>
#include <QTimer>
#include <QGraphicsRectItem>
#include "UrlItem.h"

class QGraphicsWidget;
class QGraphicsSceneResizeEvent;

class TileItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(QPointF tilePos READ tilePos WRITE setTilePos)
    Q_PROPERTY(QRectF rect READ rect WRITE setRect)
public:

    // FIXME: type should really be representing the functionality
    enum TileType {
        ThumbnailTile,
        NewWindowTile,
        EmptyWindowMarkerTile,
        ListTile
    };
    
    virtual ~TileItem();
    
    const UrlItem* urlItem() const { return &m_urlItem; }

    void setTilePos(const QPointF& pos);
    QPointF tilePos() const { return rect().topLeft(); }
    void setEditMode(bool on);
    void setFixed(bool on) { m_fixed = on; }
    bool fixed() const { return m_fixed; }
    void setContext(void* context) { m_context = context; }
    void* context() const { return m_context; }
    TileType tileType() const { return m_type; }

Q_SIGNALS:
    void itemActivated(TileItem*);
    void itemClosed(TileItem*);
    void itemLongPress(TileItem*);

protected:
    TileItem(QGraphicsWidget* parent, TileType type, const UrlItem& urlItem, bool editable = true);
    void paintExtra(QPainter* painter);
    void addDropShadow(QPainter& painter, const QRectF rect);
    void layoutTile();
    QRectF boundingRect() const;

    virtual void doLayoutTile() = 0;
    void setTileType(TileType t) { m_type = t; }

protected Q_SLOTS:
    virtual void activateItem();
    virtual void closeItem();
    void longpressTimeout();

protected:
    UrlItem m_urlItem;
    bool m_selected;
    QImage* m_closeIcon;
    QRectF m_closeIconRect;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void setEditIconRect();

private:
    TileType m_type;
    bool m_editable;
    void* m_context; 
    bool m_fixed;
    QRectF m_oldRect;
    QTimer m_longpressTimer;
    QPointF m_mousePressPos;
};

class ThumbnailTileItem : public TileItem {
    Q_OBJECT
public:
    ThumbnailTileItem(QGraphicsWidget* parent, const UrlItem& urlItem, bool editable = true);
    ~ThumbnailTileItem();
    
private:
    void doLayoutTile();
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    QString m_title;
    QRectF m_thumbnailRect;
    QRectF m_textRect;
    QImage m_defaultIcon;
    QImage m_scaledThumbnail;
};

class NewWindowTileItem : public ThumbnailTileItem {
    Q_OBJECT
public:
    NewWindowTileItem(QGraphicsWidget* parent, const UrlItem& urlItem);

public Q_SLOTS:
    void newWindowAnimFinished();

private:
    void activateItem();
    void doLayoutTile() {}
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
};

class NewWindowMarkerTileItem : public ThumbnailTileItem {
    Q_OBJECT
public:
    NewWindowMarkerTileItem(QGraphicsWidget* parent, const UrlItem& urlItem);

private:
    void doLayoutTile() {}
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
};

class ListTileItem : public TileItem {
public:
    ListTileItem(QGraphicsWidget* parent, const UrlItem& urlItem, bool editable = true);
    
private:
    void doLayoutTile();
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    QString m_title;
    QString m_url;
    QRectF m_titleRect;
    QRectF m_urlRect;
};

typedef QList<TileItem*> TileList;

#endif
