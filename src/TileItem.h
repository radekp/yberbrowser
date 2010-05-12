#ifndef TileItem_h_
#define TileItem_h_

#include <QObject>
#include <QRectF>
#include <QGraphicsRectItem>
#include <QLinearGradient>
#include "UrlItem.h"

class QGraphicsWidget;
class QGraphicsSceneResizeEvent;

class TileItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    virtual ~TileItem();
    
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    UrlItem* urlItem() const { return m_urlItem; }

    void setEditMode(bool on);
    void setContext(void* context) { m_context = context; }
    void* context() const { return m_context; }

public Q_SLOTS:
    void invalidateClick();

Q_SIGNALS:
    void itemActivated(TileItem*);
    void itemClosed(TileItem*);
    void itemEditingMode(TileItem*);

protected:
    TileItem(QGraphicsWidget* parent, UrlItem& urlItem, bool editable = true);
    void paintExtra(QPainter* painter);
    void addDropShadow(QPainter& painter, const QRectF rect);
    void layoutTile();

    virtual void doLayoutTile() = 0;

protected:
    UrlItem* m_urlItem;
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
};

class ThumbnailTileItem : public TileItem {
    Q_OBJECT
public:
    ThumbnailTileItem(QGraphicsWidget* parent, UrlItem& urlItem, bool editable = true);
    ~ThumbnailTileItem();
    
public Q_SLOTS:
    void thumbnailChanged();

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
    NewWindowTileItem(QGraphicsWidget* parent, UrlItem& urlItem);

private:
    void doLayoutTile() {}
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
};

class NewWindowMarkerTileItem : public ThumbnailTileItem {
    Q_OBJECT
public:
    NewWindowMarkerTileItem(QGraphicsWidget* parent, UrlItem& urlItem);

private:
    void doLayoutTile() {}
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
};

class ListTileItem : public TileItem {
public:
    ListTileItem(QGraphicsWidget* parent, UrlItem& urlItem, bool editable = true);
    
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
