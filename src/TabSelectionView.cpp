#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>

#include "TabSelectionView.h"
#include "UrlItem.h"
#include "HistoryStore.h"
#include "ApplicationWindow.h"
#include "WebView.h"

#include <QPropertyAnimation>

namespace {
const int s_tilePadding = 10;
}

class TabWidget : public TileBaseWidget {
    Q_OBJECT
public:
    TabWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) : TileBaseWidget("Window selection", parent, wFlags) {}

    void setActiveTabItem(TileItem* tabItem) { m_activeTabItem = tabItem; }
    QRectF activeTabItemRect();  

    void removeTile(TileItem& removed);
    void removeAll();

    void layoutTiles();

private:
    TileItem* m_activeTabItem;
};

QRectF TabWidget::activeTabItemRect()
{
    if (!m_activeTabItem)
        return QRectF(0, 0, 0, 0);

    return m_activeTabItem->rect();
}

void TabWidget::removeTile(TileItem& removed)
{
    if (m_activeTabItem == &removed)
        m_activeTabItem = 0;
    // url list is created here (out of window list) unlike in other views, like history items.
    delete removed.urlItem();
    TileBaseWidget::removeTile(removed);
}

void TabWidget::removeAll()
{
    m_activeTabItem = 0;
    // url list is created here (out of window list) unlike in other views, like history items.
    for (int i = m_tileList.size() - 1; i >= 0; --i)
        delete m_tileList.at(i)->urlItem();
    TileBaseWidget::removeAll();
}

void TabWidget::layoutTiles()
{
    // and layout them
    QRectF r(rect());

    // 6 tabs max atm
    // FIXME: this is landscape oriented. need to be dynamic to
    // support portrait mode
    int vTileNum = 2;
    int hTileNum = qMin(3, m_tileList.size());
    int tileWidth = (r.width() / 3) - 2 * s_tilePadding;
    int tileHeight = tileWidth / 1.20 - 2 * s_tilePadding;
 
    // FIXME: doLayoutTiles does horizontal centering. no need
    r.setWidth(hTileNum * tileWidth);
    doLayoutTiles(r, hTileNum, tileWidth, vTileNum, tileHeight, s_tilePadding, s_tilePadding);
}

TabSelectionView::TabSelectionView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_tabWidget(new TabWidget(this, wFlags))
    , m_windowList(0) 
    , m_activeWindow(0)
{
    m_tabWidget->setZValue(1);
    m_tabWidget->setEditMode(true);
    connect(m_tabWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

TabSelectionView::~TabSelectionView()
{
    delete m_tabWidget;
}

void TabSelectionView::setGeometry(const QRectF& rect)
{
    TileSelectionViewBase::setGeometry(rect);
    m_tabWidget->setGeometry(rect);
}

void TabSelectionView::tileItemActivated(TileItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);
    if (item->context())
        emit windowSelected((WebView*)item->context());
    else
        emit windowCreated();
}

void TabSelectionView::tileItemClosed(TileItem* item)
{
    TileSelectionViewBase::tileItemClosed(item);
    if (item->context()) {
        emit windowClosed((WebView*)item->context());
        m_tabWidget->removeTile(*item);
    }
    // close on last window close
    if (m_tabWidget->tileList()->size() == 1)
        disappear();
}

void TabSelectionView::destroyViewItems()
{
    m_tabWidget->removeAll();
}

void TabSelectionView::createViewItems()
{
    // recreate?
    destroyViewItems();

    if (!m_windowList)
        return;

    // create tile list out of window list
    for (int i = 0; i < m_windowList->size(); ++i) {
        WebView* view = m_windowList->at(i);
        bool pageAvailable = !view->url().isEmpty();
        QImage* thumbnail = 0;
    
        if (pageAvailable) {
            // get the thumbnail from history view
            if (UrlItem* item = HistoryStore::instance()->get(view->url().toString()))
                thumbnail = new QImage(*item->thumbnail());
        }
        // create a tile item with the window context set
        ThumbnailTileItem* newTileItem = new ThumbnailTileItem(m_tabWidget, *(new UrlItem(view->url(), pageAvailable ? view->title() : "no page loded yet", thumbnail)));
        newTileItem->setContext(view);

        m_tabWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
        // remember the active window so that we can scroll to
        if (view == m_activeWindow)
            m_tabWidget->setActiveTabItem(newTileItem);
    }
    
    NewWindowTileItem* newTileItem = new NewWindowTileItem(m_tabWidget, *(new UrlItem(QUrl(), "", 0)));
    m_tabWidget->addTile(*newTileItem);
    connectItem(*newTileItem);

    m_tabWidget->layoutTiles();
}

#include "TabSelectionView.moc"
