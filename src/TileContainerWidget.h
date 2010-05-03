#ifndef TileContainerWidget_h_
#define TileContainerWidget_h_

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"
#include "TileItem.h"

class QGraphicsSceneMouseEvent;
class QParallelAnimationGroup;

class PannableTileContainer : public PannableViewport, private CommonGestureConsumer {
    Q_OBJECT
public:
    PannableTileContainer(QGraphicsItem*, Qt::WindowFlags wFlags = 0);
    ~PannableTileContainer();
    
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
};

class TileBaseWidget : public QGraphicsWidget {
    Q_OBJECT
public:
    ~TileBaseWidget();

    virtual void addTile(TileItem& newItem);
    virtual void removeTile(TileItem& removed);
    virtual void removeAll();

    void setEditMode(bool on);
    bool editMode() const { return m_editMode; }

    TileList* tileList() { return &m_tileList; }

    virtual void layoutTiles() = 0;

Q_SIGNALS:
    void closeWidget();

protected:
    TileBaseWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);

    virtual void doLayoutTiles(const QRectF& rect, int vTileNum, int tileWidth, int hTileNum, int tileHeight, int paddingX, int paddingY);

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    void addMoveAnimation(TileItem& item, int delay, const QRectF& oldPos, const QRectF& newPos);

protected:
    TileList m_tileList;

private:
    QParallelAnimationGroup* m_slideAnimationGroup;
    bool m_editMode;
};

#endif
