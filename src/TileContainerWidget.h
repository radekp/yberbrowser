#ifndef TileContainerWidget_h_
#define TileContainerWidget_h_

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"
#include <QList>
#include "UrlItem.h"
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

    virtual void setupWidgetContent() = 0;
    void destroyWidgetContent();

    void setEditMode(bool on);
    bool editMode() const { return m_editMode; }
    void removeTile(UrlItem& removed);

Q_SIGNALS:
    void closeWidget();

protected:
    TileBaseWidget(UrlList*, QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void addTiles(const QRectF& rect, int vTileNum, int tileWidth, int hTileNum, int tileHeight, int paddingX, int paddingY, TileItem::TileLayout layout);

protected:
    const UrlList* m_urlList;
    QList<TileItem*> m_tileList;

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    void addMoveAnimation(TileItem& item, int delay, const QRectF& oldPos, const QRectF& newPos);

private:
    QParallelAnimationGroup* m_slideAnimationGroup;
    bool m_editMode;
};

#endif
