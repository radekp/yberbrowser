#ifndef HomeView_h_
#define HomeView_h_

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"
#include <QList>
#include "UrlStore.h"

class TileItem;
class QGraphicsRectItem;
class QPropertyAnimation;
class UrlItem;
class ApplicationWindow;
class HomeContainer;
class HistoryContainer;
class BookmarkContainer;
class QGraphicsSceneMouseEvent;

class HomeView : public PannableViewport, private CommonGestureConsumer {
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

protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);
    void cancelLeftMouseButtonPress(const QPoint&);

    void mousePressEventFromChild(QGraphicsSceneMouseEvent*);
    void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent*) {}
    void adjustClickPosition(QPointF&) {}

private:
    void startAnimation(bool);

private:
    QGraphicsRectItem* m_bckg;
    HomeContainer* m_homeContainer;
    QPropertyAnimation* m_slideAnim;
    bool m_active;
    CommonGestureRecognizer m_recognizer;
    QGraphicsSceneMouseEvent* m_selfSentEvent;
};

class HomeContainer : public QGraphicsWidget {
    Q_OBJECT
public:
    HomeContainer(HomeView*, Qt::WindowFlags wFlags = 0);
    ~HomeContainer();

    void setGeometry(const QRectF& rect);

    void createViewItems();
    void destroyViewItems();

private:
    HistoryContainer* m_historyContainer;
    BookmarkContainer* m_bookmarkContainer;
};

class TileContainer : public QGraphicsWidget {
    Q_OBJECT
public:
    TileContainer(UrlList&, QGraphicsItem*, Qt::WindowFlags wFlags = 0);
    ~TileContainer();
    
    virtual void createTiles() = 0;
    void destroyTiles();

protected:
    void addTiles(int vTileNum, int tileWidth, int hTileNum, int tileHeight, int paddingX, int paddingY, bool textOnly);

private:
    UrlList* m_urlList;
    QList<TileItem*> m_tileList;
};

class HistoryContainer : public TileContainer {
    Q_OBJECT
public:
    HistoryContainer(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void createTiles();
};

class BookmarkContainer : public TileContainer {
    Q_OBJECT
public:
    BookmarkContainer(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void createTiles();

private:
    UrlList m_list;
};

#endif
