#ifndef HomeView_h_
#define HomeView_h_

#include "TileSelectionViewBase.h"

class HistoryWidget;
class BookmarkWidget;
class PannableTileContainer;
class TileItem;
class QGraphicsSceneMouseEvent;

class HomeView : public TileSelectionViewBase {
    Q_OBJECT
public:
    HomeView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HomeView();

    void setGeometry(const QRectF& rect);
    bool sceneEventFilter(QGraphicsItem*, QEvent*);

    bool recognizeFlick(QGraphicsSceneMouseEvent* e);

Q_SIGNALS:
    void pageSelected(const QUrl&);

private Q_SLOTS:
    void tileItemActivated(TileItem*);
    void tileItemClosed(TileItem*);
    void tileItemEditingMode(TileItem*);

private:
    void moveViews(bool swap);

    bool setupInAndOutAnimation(bool);
    void createViewItems();
    void destroyViewItems();

    void createBookmarkContent();
    void createHistoryContent();

private:
    BookmarkWidget* m_bookmarkWidget;
    HistoryWidget* m_historyWidget;
    PannableTileContainer* m_pannableContainer;

    // FIXME these should go to a gesture recognizer
    bool m_mouseDown;
    QPointF m_mousePos;
    int m_hDelta;
};

#endif
