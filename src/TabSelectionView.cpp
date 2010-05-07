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
#if USE_DUI
#include <DuiScene>
#endif

// FIXME: this is a complete fixme class

namespace {
const int s_tilePadding = 20;
}

class TabWidget : public TileBaseWidget {
    Q_OBJECT
public:
    TabWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) : TileBaseWidget(parent, wFlags) {}

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
    QRectF r(parentWidget()->rect());

    // default 
    int hTileNum = m_tileList.size();
    int tileWidth = (r.width() / 3) - s_tilePadding;
    int tileHeight = tileWidth / 1.20;

    r.setWidth((tileWidth + s_tilePadding) * hTileNum);
    // the width of the view is unknow until we figure out how many items there are
    setGeometry(r);

    // move tiles to the middle
    r.setTop(r.center().y() - (tileHeight / 2));
    r.setHeight(tileHeight);
    doLayoutTiles(r, hTileNum, tileWidth, 1, tileHeight, s_tilePadding, 0);
}

TabSelectionView::TabSelectionView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_pannableTabContainer(new PannableTileContainer(this, wFlags))
    , m_tabWidget(new TabWidget(this, wFlags))
    , m_windowList(0) 
    , m_activeWindow(0)
{
    m_tabWidget->setZValue(1);
    m_tabWidget->setEditMode(true);
    m_pannableTabContainer->setWidget(m_tabWidget);
    connect(m_tabWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

TabSelectionView::~TabSelectionView()
{
    delete m_tabWidget;
    delete m_pannableTabContainer;
}

void TabSelectionView::setGeometry(const QRectF& rect)
{
    QRectF currentRect(geometry());
    if (rect == currentRect)
        return;
    
    if (rect.width() != currentRect.width() || rect.height() != currentRect.height()) {
        // readjust subcontainers' sizes in case of size chage
        m_pannableTabContainer->setGeometry(rect);
        m_tabWidget->setGeometry(rect);
    }
    TileSelectionViewBase::setGeometry(rect);
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

void TabSelectionView::setupAnimation(bool in)
{
    // animate all the way down to the current window
    QPropertyAnimation* tabAnim = new QPropertyAnimation(m_tabWidget, "geometry");
    tabAnim->setDuration(800);
    QRectF r(m_tabWidget->geometry());

    if (in) {
        // scroll all the way down to the active item
        QRectF finishRect(m_tabWidget->activeTabItemRect());
        r.moveLeft(finishRect.left() > 0 ? rect().center().x() - (finishRect.left() + finishRect.width()/2) : rect().left());
    }
    QRectF hiddenWidget(r); hiddenWidget.moveLeft(r.right());

    tabAnim->setStartValue(in ?  hiddenWidget : r);
    tabAnim->setEndValue(in ? r : hiddenWidget);

    tabAnim->setEasingCurve(in ? QEasingCurve::OutBack : QEasingCurve::InCubic);

    m_slideAnimationGroup->addAnimation(tabAnim);

    // hide the container
    if (in)
        m_tabWidget->setGeometry(hiddenWidget);
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
        TileItem* newTileItem = new TileItem(m_tabWidget, *(new UrlItem(view->url(), pageAvailable ? view->title() : "no page loded yet", thumbnail)), TileItem::Vertical);
        newTileItem->setContext(view);

        m_tabWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
        // remember the active window so that we can scroll to
        if (view == m_activeWindow)
            m_tabWidget->setActiveTabItem(newTileItem);
    }
    TileItem* newTileItem = new TileItem(m_tabWidget, *(new UrlItem(QUrl(), "open new window", 0)), TileItem::Vertical, false);
    m_tabWidget->addTile(*newTileItem);
    connectItem(*newTileItem);

    m_tabWidget->layoutTiles();
}

#include "TabSelectionView.moc"
