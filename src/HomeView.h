#ifndef HomeView_h_
#define HomeView_h_

#include "TileSelectionViewBase.h"

class HistoryWidget;
class BookmarkWidget;
class PannableTileContainer;
class UrlItem;

class HomeView : public TileSelectionViewBase {
    Q_OBJECT
public:
    HomeView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HomeView();

    void setGeometry(const QRectF& rect);

Q_SIGNALS:
    void urlSelected(const QUrl&);

public Q_SLOTS:
    void tileItemActivated(UrlItem*);
    void tileItemEdited(UrlItem*);

private:
    void setupAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    HistoryWidget* m_historyWidget;
    BookmarkWidget* m_bookmarkWidget;
    PannableTileContainer* m_pannableHistoryContainer;
};

#endif
