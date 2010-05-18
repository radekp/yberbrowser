#ifndef HomeView_h_
#define HomeView_h_

#include "TileSelectionViewBase.h"
#include <QTime>

class HistoryWidget;
class BookmarkWidget;
class TabWidget;
class PannableTileContainer;
class TileItem;
class QGraphicsSceneMouseEvent;
class WebView;
class TileBaseWidget;

class HomeView : public TileSelectionViewBase {
    Q_OBJECT
public:
    enum HomeWidgetType {
        WindowSelect,
        VisitedPages,
        Bookmarks
    };
   
    HomeView(HomeWidgetType initialWidget, QPixmap* bckg, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HomeView();
    
    void setWindowList(QList<WebView*>& windowList) { m_windowList = &windowList; }
    HomeWidgetType activeWidget() const { return m_activeWidget; }
    void setActiveWidget(HomeWidgetType widget);
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    bool recognizeFlick(QGraphicsSceneMouseEvent* e);

Q_SIGNALS:
    void pageSelected(const QUrl&);
    void windowSelected(WebView* webView);
    void windowClosed(WebView* webView);
    void windowCreated(bool);

private Q_SLOTS:
    void tileItemActivated(TileItem*);
    void tileItemClosed(TileItem*);
    void tileItemEditingMode(TileItem*);

private:
    void moveViews();

    void createViewItems();
    void destroyViewItems();

    void createBookmarkContent();
    void createHistoryContent();
    void createTabSelectContent();

    TileBaseWidget* widgetByType(HomeWidgetType type);
    PannableTileContainer* activePannableContainer();

private:
    HomeWidgetType m_activeWidget;
    BookmarkWidget* m_bookmarkWidget;
    HistoryWidget* m_historyWidget;
    TabWidget* m_tabWidget;
    PannableTileContainer* m_pannableHistoryContainer;
    PannableTileContainer* m_pannableBookmarkContainer;
    PannableTileContainer* m_pannableWindowSelectContainer;
    QList<WebView*>* m_windowList;

    // FIXME these should go to a gesture recognizer
    QTime m_flickTime;
    bool m_horizontalFlickLocked;
    bool m_mouseDown;
    QPointF m_mousePos;
    int m_hDelta;
};

#endif
