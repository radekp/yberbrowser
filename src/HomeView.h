#ifndef HomeView_h_
#define HomeView_h_

#include "TileSelectionViewBase.h"

class HistoryWidget;
class BookmarkWidget;
class PannableTileContainer;
class TileItem;

class HomeView : public TileSelectionViewBase {
    Q_OBJECT
public:
    HomeView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HomeView();

    void setGeometry(const QRectF& rect);

Q_SIGNALS:
    void pageSelected(const QUrl&);

public Q_SLOTS:
    void tileItemActivated(TileItem*);
    void tileItemClosed(TileItem*);
    void tileItemEditingMode(TileItem*);

private:
    bool setupAnimation(bool);
    void createViewItems();
    void destroyViewItems();

    void createBookmarkContent();
    void createHistoryContent();

private:
    BookmarkWidget* m_bookmarkWidget;
    HistoryWidget* m_historyWidget;
    PannableTileContainer* m_pannableHistoryContainer;
};

#endif
