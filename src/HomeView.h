#ifndef HomeView_h_
#define HomeView_h_

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"
#include <QList>
#include "UrlItem.h"
#include "TileItem.h"

class QGraphicsRectItem;
class ApplicationWindow;
class PannableTileContainer;
class HistoryWidget;
class BookmarkWidget;
class QGraphicsSceneMouseEvent;
class QParallelAnimationGroup;

class HomeView : public QGraphicsWidget {
    Q_OBJECT
public:
    HomeView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HomeView();

    void setGeometry(const QRectF& rect);

    bool isActive() const { return m_active; }
    void appear(ApplicationWindow*);
    void disappear();

Q_SIGNALS:
    void appeared();
    void disappeared();
    void urlSelected(const QUrl&);

public Q_SLOTS:
    void animFinished();
    void tileItemActivated(UrlItem*);

private:
    void startAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    QGraphicsRectItem* m_bckg;
    PannableTileContainer* m_pannableHistoryContainer;
    HistoryWidget* m_historyWidget;
    BookmarkWidget* m_bookmarkWidget;
    QParallelAnimationGroup* m_slideAnimationGroup;
    bool m_active;
};

class PannableTileContainer : public PannableViewport, private CommonGestureConsumer {
    Q_OBJECT
public:
    PannableTileContainer(QGraphicsItem*, Qt::WindowFlags wFlags = 0);
    ~PannableTileContainer();
    
protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);
    void cancelLeftMouseButtonPress(const QPoint&);

    void mousePressEventFromChild(QGraphicsSceneMouseEvent*);
    void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent*) {}
    void adjustClickPosition(QPointF&) {}

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

protected:
    TileBaseWidget(const UrlList&, QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void addTiles(const QRectF& rect, int vTileNum, int tileWidth, int hTileNum, int tileHeight, int paddingX, int paddingY, TileItem::TileLayout layout);

protected:
    const UrlList* m_urlList;
    QList<TileItem*> m_tileList;
};

class HistoryWidget : public TileBaseWidget {
    Q_OBJECT
public:
    HistoryWidget(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void setupWidgetContent();
};

class BookmarkWidget : public TileBaseWidget {
    Q_OBJECT
public:
    BookmarkWidget(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void setupWidgetContent();

private:
    QGraphicsRectItem* m_bckg;
    QLinearGradient m_bckgGradient;
};

#endif
