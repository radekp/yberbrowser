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
#include <QGraphicsRectItem>
#include "UrlItem.h"

class QGraphicsWidget;
class QGraphicsSceneResizeEvent;

class TileItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(QPointF tilePos READ tilePos WRITE setTilePos)
    Q_PROPERTY(QRectF rect READ rect WRITE setRect)
public:
    virtual ~TileItem();
    
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    UrlItem* urlItem() { return &m_urlItem; }

    void setTilePos(const QPointF& pos) { m_dirty = true; setRect(QRectF(pos, rect().size())); }
    QPointF tilePos() const { return rect().topLeft(); }
    void setEditMode(bool on);
    void setFixed(bool on) { m_fixed = on; }
    bool fixed() const { return m_fixed; }
    void setContext(void* context) { m_context = context; }
    void* context() const { return m_context; }

Q_SIGNALS:
    void itemActivated(TileItem*);
    void itemClosed(TileItem*);
    void itemEditingMode(TileItem*);

protected:
    TileItem(QGraphicsWidget* parent, const UrlItem& urlItem, bool editable = true);
    void paintExtra(QPainter* painter);
    void addDropShadow(QPainter& painter, const QRectF rect);
    void layoutTile();

    virtual void doLayoutTile() = 0;

protected Q_SLOTS:
    void invalidateClick();
    virtual void activateItem();
    virtual void closeItem();

protected:
    UrlItem m_urlItem;
    bool m_selected;
    QImage* m_closeIcon;
    QRectF m_closeIconRect;

private:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    void setEditIconRect();

private:
    bool m_dclick;
    bool m_editable;
    void* m_context; 
    bool m_dirty;
    bool m_fixed;
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
    QImage* m_defaultIcon;
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
