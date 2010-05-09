#ifndef TileContainerWidget_h_
#define TileContainerWidget_h_

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"
#include "TileItem.h"

const int s_viewMargin = 40;

class QGraphicsSceneMouseEvent;
class QParallelAnimationGroup;
class HomeView;

class PannableTileContainer : public PannableViewport, private CommonGestureConsumer {
    Q_OBJECT
public:
    PannableTileContainer(QGraphicsItem*, Qt::WindowFlags wFlags = 0);
    ~PannableTileContainer();
    
    void setHomeView(HomeView* view) { m_homeView = view; }
protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);
    void cancelLeftMouseButtonPress(const QPoint&);

    void mousePressEventFromChild(QGraphicsSceneMouseEvent*);
    void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent*);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent*);
    void adjustClickPosition(QPointF&) {}

private:
    void forwardEvent(QGraphicsSceneMouseEvent*);

private:
    CommonGestureRecognizer m_recognizer;
    QGraphicsSceneMouseEvent* m_selfSentEvent;
    // FIXME: remove it once the event handling is fixed
    HomeView* m_homeView;
};

class TileBaseWidget : public QGraphicsWidget {
    Q_OBJECT
public:
    ~TileBaseWidget();

    virtual void addTile(TileItem& newItem);
    virtual void removeTile(TileItem& removed);
    virtual void removeAll();

    void setActive(bool active) { m_active = active; }
    bool active() const { return m_active; }

    void setEditMode(bool on);
    bool editMode() const { return m_editMode; }

    TileList* tileList() { return &m_tileList; }

    virtual void layoutTiles() = 0;

Q_SIGNALS:
    void closeWidget();
    void deactivateWidget();

protected:
    TileBaseWidget(const QString& title, QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);

    virtual void doLayoutTiles(const QRectF& rect, int vTileNum, int tileWidth, int hTileNum, int tileHeight, int paddingX, int paddingY);

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    void addMoveAnimation(TileItem& item, int delay, const QRectF& oldPos, const QRectF& newPos);

protected:
    TileList m_tileList;

private:
    QParallelAnimationGroup* m_slideAnimationGroup;
    QString m_title;
    bool m_editMode;
    bool m_active;
    int m_maxTileCount;
};

#endif
