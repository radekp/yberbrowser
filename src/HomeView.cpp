#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsPixmapItem>
#include <QTimer>

#include "HomeView.h"
#include "TileContainerWidget.h"
#include "TileItem.h"
#include "HistoryStore.h"
#include "BookmarkStore.h"
#include "ApplicationWindow.h"
#include "WebView.h"
#if USE_DUI
#include <DuiScene>
#endif

// FIXME: HomeView should be either a top level view, or just a central widget of the browsingview, probably the first one
// also, LAF is not finalized yet.

const int s_containerMargin = 40;
const int s_tileMargin = 20;
const int s_historyTileMargin = 30;
const int s_bookmarksTileHeight = 70;
const int s_maxHistoryTileNum = 20;
const int s_maxWindows = 6;

class TabWidget : public TileBaseWidget {
    Q_OBJECT
public:
    TabWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) : TileBaseWidget("Window selection", parent, wFlags) {}
    void layoutTiles();

    void removeTile(TileItem& removed);
    void removeAll();
};

void TabWidget::removeTile(TileItem& removed)
{
    // FIXME: when tab is full, fake items dont work
    // insert a fake marker item in place
    NewWindowMarkerTileItem* emptyItem = new NewWindowMarkerTileItem(this, *(new UrlItem(QUrl(), "", 0)));
    for (int i = 0; i < m_tileList.size(); ++i) {
        if (m_tileList.at(i)->fixed()) {
            emptyItem->setRect(m_tileList.at(i-1)->rect());
            m_tileList.insert(i, emptyItem);
            break;
        }            
    }
    // url list is created here (out of window list) unlike in other views, like history items.
    delete removed.urlItem();
    TileBaseWidget::removeTile(removed);
}

void TabWidget::removeAll()
{
    // url list is created here (out of window list) unlike in other views, like history items.
    for (int i = m_tileList.size() - 1; i >= 0; --i)
        delete m_tileList.at(i)->urlItem();
    TileBaseWidget::removeAll();
}

void TabWidget::layoutTiles()
{
    // 6 tabs max atm
    // FIXME: this is landscape oriented. check aspect ratio
    QRectF r(rect());
    r.setHeight(r.height() - s_viewMargin);
    doLayoutTiles(r, 3, 2, s_tileMargin, s_tileMargin, true);
}

class HistoryWidget : public TileBaseWidget {
    Q_OBJECT
public:
    HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) :TileBaseWidget("Visited pages", parent, wFlags) {}
    void layoutTiles();
};

void HistoryWidget::layoutTiles()
{
    // the height of the view is unknow until we layout the tiles
    QRectF r(rect());
    r.setHeight(parentWidget()->size().height() - s_viewMargin);
    // FIXME: this is landscape oriented. check aspect ratio
    setMinimumHeight(doLayoutTiles(r, 3, 2, s_historyTileMargin, s_historyTileMargin).height()); 
}

class BookmarkWidget : public TileBaseWidget {
    Q_OBJECT
public:
    BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) :TileBaseWidget("Bookmarks", parent, wFlags) {}
    void layoutTiles();
};

void BookmarkWidget::layoutTiles()
{
    // the height of the view is unknow until we layout the tiles
    QRectF r(rect());
    r.setHeight(parentWidget()->size().height() - s_viewMargin);
    setMinimumHeight(doLayoutTiles(r, 1, r.height()/s_bookmarksTileHeight, s_tileMargin, 0).height());
}

HomeView::HomeView(HomeWidgetType initialWidget, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(TileSelectionViewBase::Home, parent, wFlags)
    , m_activeWidget(initialWidget)
    , m_bookmarkWidget(new BookmarkWidget(this, wFlags))
    , m_historyWidget(new HistoryWidget(this, wFlags))
    , m_tabWidget(new TabWidget(this, wFlags))
    , m_pannableHistoryContainer(new PannableTileContainer(this, wFlags))
    , m_pannableBookmarkContainer(new PannableTileContainer(this, wFlags))
    , m_windowList(0) 
{
    setFiltersChildEvents(true);
    m_pannableHistoryContainer->setHomeView(this);
    m_pannableHistoryContainer->setWidget(m_historyWidget);
    m_pannableBookmarkContainer->setHomeView(this);
    m_pannableBookmarkContainer->setWidget(m_bookmarkWidget);
    
    m_tabWidget->setEditMode(true);

    connect(m_tabWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
    connect(m_bookmarkWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
    connect(m_historyWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

HomeView::~HomeView()
{
    delete m_tabWidget;
    delete m_pannableHistoryContainer;
    delete m_pannableBookmarkContainer;
}

void HomeView::setActiveWidget(HomeWidgetType widget)
{
    if (widget == m_activeWidget)
        return;

    m_activeWidget = widget;
    int containerWidth = rect().width() / 3 - s_containerMargin;

    // repositon view
    if (m_activeWidget == WindowSelect)
        setPos(0, 0);
    else if (m_activeWidget == VisitedPages)
        setPos(QPointF(-(containerWidth - s_containerMargin), 0));
    else if (m_activeWidget == Bookmarks)
        setPos(QPointF(-(2*containerWidth - 2*s_containerMargin), 0));
}

bool HomeView::sceneEventFilter(QGraphicsItem* /*i*/, QEvent* e)
{
    bool doFilter = false;

    if (!isVisible())
        return doFilter;

    switch (e->type()) {
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick:
        doFilter = recognizeFlick(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;
    default:
        break;
    }
    return doFilter;
}

bool HomeView::recognizeFlick(QGraphicsSceneMouseEvent* e)
{
    if (e->type() == QEvent::GraphicsSceneMousePress) {
        m_mouseDown = true;
        m_mousePos = e->scenePos();
        m_hDelta = 0;
    } else if (e->type() == QEvent::GraphicsSceneMouseRelease) {
        m_mouseDown = false;
        // swap, move back, or no move at all?
        if (abs(m_hDelta) > 0) {
            moveViews();
            return true;
        }
    } if (e->type() == QEvent::GraphicsSceneMouseMove && m_mouseDown) {
        // panning
        QPointF delta(m_mousePos - e->scenePos());
        if (abs(delta.x()) >= abs(delta.y())) {
            QPointF p(pos());
            p.setX(p.x() - delta.x());
            if (p.x() > 0)
                p.setX(0);
            else if (abs(p.x()) > size().width()/3*2)
                p.setX(-(size().width()/3*2));            

            m_mousePos = e->scenePos();
            m_hDelta+=delta.x();
            setPos(p);
            return true;
        }
    }
    return false;
}

void HomeView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QRectF r(rect()); 
    int containerWidth = r.width() / 3 - s_containerMargin;

    // tab to the leftmost pos
    r.setWidth(containerWidth);
    m_tabWidget->setGeometry(r);
    if (m_activeWidget == WindowSelect)
        setPos(-r.topLeft());

    // move history to the middle
    r.moveLeft(containerWidth); 
    r.setWidth(containerWidth - s_containerMargin);
    m_pannableHistoryContainer->setGeometry(r);
    m_historyWidget->setGeometry(QRectF(QPointF(0, 0), r.size()));
    if (m_activeWidget == VisitedPages)
        setPos(QPointF(-(r.x() - s_containerMargin), 0));

    // rigthmost
    r.moveLeft(2*containerWidth - s_containerMargin); 
    m_pannableBookmarkContainer->setGeometry(r);
    m_bookmarkWidget->setGeometry(QRectF(QPointF(0, 0), r.size()));
    if (m_activeWidget == Bookmarks)
        setPos(QPointF(-(r.x() - s_containerMargin), 0));
    TileSelectionViewBase::resizeEvent(event);
}

void HomeView::tileItemActivated(TileItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);
    if (m_activeWidget == WindowSelect) {
        if (item->context())
            emit windowSelected((WebView*)item->context());
        else {
            emit windowCreated(true);
        }
    } else {
        emit pageSelected(item->urlItem()->m_url);
    }
}

void HomeView::tileItemClosed(TileItem* item)
{
    TileSelectionViewBase::tileItemClosed(item);
    widgetByType(m_activeWidget)->removeTile(*item);

    if (m_activeWidget == WindowSelect)
        emit windowClosed((WebView*)item->context());
}

void HomeView::tileItemEditingMode(TileItem* item)
{
    TileSelectionViewBase::tileItemEditingMode(item);

    TileBaseWidget* w = widgetByType(m_activeWidget); 
    w->setEditMode(!w->editMode());
    update();
}

void HomeView::moveViews()
{
    // check which container wins
    m_slideAnimationGroup->stop();
    m_slideAnimationGroup->clear();

    int viewWidth = size().width();
    int containerWidth = viewWidth / 3 - s_containerMargin;
    // at what pos it should jump to the next container
    int goOverWidth = containerWidth / 2;
    // since go over is happening at the half of the container ->
    // newContainer = 0 first, left container, 1,2 ->middle 3->last, right
    int newContainer = abs(pos().x() / goOverWidth);
    int newPos = 0;
    // rightmost >> middle >> leftmost
    if (newContainer >= 3) {
        newPos = 2 * containerWidth - 2*s_containerMargin;
        m_activeWidget = Bookmarks;
    } else if (newContainer > 0) {
        newPos = containerWidth - s_containerMargin;
        m_activeWidget = VisitedPages;
    } else {
        m_activeWidget = WindowSelect;
    }

    QRectF to(geometry());
    to.moveLeft(-newPos);

    QPropertyAnimation* slideAnim = new QPropertyAnimation(this, "geometry");
    slideAnim->setStartValue(geometry());
    slideAnim->setEndValue(to);

    slideAnim->setDuration(500);
    slideAnim->setEasingCurve(QEasingCurve::OutExpo);
    m_slideAnimationGroup->addAnimation(slideAnim);
    m_slideAnimationGroup->start(QAbstractAnimation::KeepWhenStopped);
}

void HomeView::destroyViewItems()
{
    m_tabWidget->removeAll();
    m_historyWidget->removeAll();
    m_bookmarkWidget->removeAll();
}

void HomeView::createViewItems()
{
    // recreate?
    destroyViewItems();
    //
    createTabSelectContent();
    createHistoryContent();
    createBookmarkContent();
}

void HomeView::createBookmarkContent()
{
    UrlList* list = BookmarkStore::instance()->list();
    for (int i = 0; i < list->size(); ++i) {
        ListTileItem* newTileItem = new ListTileItem(m_bookmarkWidget, *list->at(i));
        m_bookmarkWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    m_bookmarkWidget->layoutTiles();
}

void HomeView::createHistoryContent()
{
    ThumbnailTileItem* newTileItem = 0;
    UrlList* list = HistoryStore::instance()->list();
    for (int i = 0; i < (s_maxHistoryTileNum - 1) && i < list->size(); ++i) {
        newTileItem = new ThumbnailTileItem(m_historyWidget, *list->at(i));
        m_historyWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    // FIXME: this 'All history'  is being leaked
    newTileItem = new ThumbnailTileItem(m_historyWidget, *(new UrlItem(QUrl(), "All history", 0)), false);
    m_historyWidget->addTile(*newTileItem);
    connectItem(*newTileItem);

    m_historyWidget->layoutTiles();
}

void HomeView::createTabSelectContent()
{
    if (!m_windowList)
        return;

    // create tile list out of window list
    int i = 0;
    for (; i < m_windowList->size(); ++i) {
        WebView* view = m_windowList->at(i);
        bool pageAvailable = !view->url().isEmpty();
        QImage* thumbnail = 0;
    
        if (pageAvailable) {
            // get the thumbnail from history store, it'd better be there
            if (UrlItem* item = HistoryStore::instance()->get(view->url().toString()))
                thumbnail = new QImage(*item->thumbnail());
        }
        // create a tile item with the window context set
        ThumbnailTileItem* newTileItem = new ThumbnailTileItem(m_tabWidget, *(new UrlItem(view->url(), pageAvailable ? view->title() : "no page loded yet", thumbnail)));
        newTileItem->setContext(view);

        m_tabWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    
    NewWindowTileItem* newTileItem = new NewWindowTileItem(m_tabWidget, *(new UrlItem(QUrl(), "", 0)));
    m_tabWidget->addTile(*newTileItem);
    i++;
    connectItem(*newTileItem);
    
    for (; i < s_maxWindows; i++) {
        NewWindowMarkerTileItem* emptyItem = new NewWindowMarkerTileItem(m_tabWidget, *(new UrlItem(QUrl(), "", 0)));
        m_tabWidget->addTile(*emptyItem);
    }

    m_tabWidget->layoutTiles();
}

TileBaseWidget* HomeView::widgetByType(HomeWidgetType type)
{
    if (type == WindowSelect)
        return m_tabWidget;
    else if (type == VisitedPages)
        return m_historyWidget;
    else if (type == Bookmarks)
        return m_bookmarkWidget;
    return 0;    
}

#include "HomeView.moc"
