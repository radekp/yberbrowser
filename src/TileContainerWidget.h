#ifndef TileContainerWidget_h_
#define TileContainerWidget_h_

#include <QGraphicsWidget>
#include "TileItem.h"

class QGraphicsSceneMouseEvent;
class QParallelAnimationGroup;
class HomeView;

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
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    void addMoveAnimation(TileItem& item, int delay, const QPointF& oldPos, const QPointF& newPos);

protected:
    TileList m_tileList;

private:
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
    void removeAll();
};

class HistoryWidget : public TileBaseWidget {
    Q_OBJECT
public:
    HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);
    void layoutTiles();
};

class BookmarkWidget : public TileBaseWidget {
    Q_OBJECT
public:
    BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);
    void layoutTiles();
};

class PopupWidget : public TileBaseWidget {
    Q_OBJECT
public:
    PopupWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);
    void layoutTiles();
};

#endif
