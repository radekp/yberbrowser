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
    TabWidget(UrlList*, QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void setActiveTabItem(UrlItem* tabItem) { m_activeTabItem = tabItem; }
    QRectF activeTabItemRect();  

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void setupWidgetContent();

private:
    UrlItem* m_activeTabItem;
};

TabWidget::TabWidget(UrlList* tabList, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileBaseWidget(tabList, parent, wFlags)
{
}

QRectF TabWidget::activeTabItemRect()
{
    if (!m_activeTabItem)
        return QRectF(0, 0, 0, 0);

    for(int i = 0; i < m_tileList.size(); ++i) {
        if (m_tileList.at(i)->urlItem() == m_activeTabItem)
            return m_tileList.at(i)->rect();
    }
    return QRectF(0, 0, 0, 0);
}

void TabWidget::setupWidgetContent()
{
    int width = parentWidget()->geometry().width();

    // default 
    int hTileNum = m_urlList->size();
    int tileWidth = (width / 3) - s_tilePadding;
    int tileHeight = tileWidth / 1.20;
    
    QRectF r(rect());
    r.setWidth((tileWidth + s_tilePadding) * hTileNum);
    setGeometry(r);

    // move tiles to the middle
    r.setTop(r.center().y() - (tileHeight / 2));
    r.setHeight(tileHeight);

    addTiles(r, hTileNum, tileWidth, 1, tileHeight, s_tilePadding, 0, TileItem::Vertical);
}

void TabWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    TileBaseWidget::paint(painter, option, widget);
}

TabSelectionView::TabSelectionView(QList<WebView*>& windowList, WebView* activeWindow, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_pannableTabContainer(new PannableTileContainer(this, wFlags))
    , m_tabWidget(new TabWidget(&m_tabList, this, wFlags))
    , m_windowList(&windowList) 
{
    // create url list out of window list
    for (int i = 0; i < windowList.size(); ++i) {
        WebView* view = windowList.at(i);
        bool pageAvailable = !view->url().isEmpty();
        QImage* thumbnail = 0;
    
        if (pageAvailable) {
            // get the thumbnail from history view
            if (UrlItem* item = HistoryStore::instance()->get(view->url().toString()))
                thumbnail = item->thumbnail();      
        }

        UrlItem* newItem = new UrlItem(view->url(), pageAvailable ? view->title() : "no page loded yet", thumbnail, view);
        m_tabList.append(newItem);
        // remember the active window so that we can scroll to
        if (view == activeWindow)
            m_tabWidget->setActiveTabItem(newItem);
    }
    m_tabList.append(new UrlItem(QUrl(), "open new window", 0, 0));
    m_tabWidget->setZValue(1);
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

void TabSelectionView::tileItemActivated(UrlItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);
    if (!item->m_context)
        emit createNewWindow();
    else
        emit windowSelected((WebView*)item->m_context);
}

void TabSelectionView::tileItemEdited(UrlItem* item)
{
    TileSelectionViewBase::tileItemEdited(item);
    if (item->m_context) {
        emit windowClosed((WebView*)item->m_context);
        m_tabWidget->removeTile(*item);
    }
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
    m_tabWidget->destroyWidgetContent();
}

void TabSelectionView::createViewItems()
{
   m_tabWidget->setupWidgetContent();
    m_tabWidget->setEditMode(true);
}

#include "TabSelectionView.moc"
