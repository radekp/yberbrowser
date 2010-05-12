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

#endif
