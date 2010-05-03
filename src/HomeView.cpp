#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>

#include "HomeView.h"
#include "TileContainerWidget.h"
#include "TileItem.h"
#include "HistoryStore.h"
#include "BookmarkStore.h"
#include "ApplicationWindow.h"

#include <QPropertyAnimation>
#if USE_DUI
#include <DuiScene>
#endif

// FIXME: HomeView should be either a top level view, or just a central widget of the browsingview, probably the first one
// also, LAF is not finalized yet.

namespace {
const int s_minTileWidth = 170;
const int s_maxTileWidth = 200;
const int s_defaultTileNumH = 3;
const int s_defaultTileNumV = 6;
const int s_tileHistoryPadding = 10;
const int s_tileBookmarkPadding = 0;
const int s_bookmarkStripeHeight = 70;
const int s_bookmarksTileWidth = 135;
const QColor s_TitleTextColor(0xFA, 0xFA, 0xFA);

const int s_maxHistoryTileNum = s_defaultTileNumH * s_defaultTileNumV;
}

class HistoryWidget : public TileBaseWidget {
    Q_OBJECT
public:
    HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) :TileBaseWidget(parent, wFlags) {}

    void layoutTiles();
};

void HistoryWidget::layoutTiles()
{
    int width = rect().width();

    if (width < s_minTileWidth)
        return;

    // default 
    int hTileNum = s_defaultTileNumH;
    int tileWidth = width / hTileNum;

    // calculate the optimal tile width
    while (tileWidth < s_minTileWidth && hTileNum > 0)
        tileWidth = width / --hTileNum;

    tileWidth = qMin(tileWidth, s_maxTileWidth);

    // keep height proposional
    int vTileNum = s_defaultTileNumV;
    int tileHeight = tileWidth * 0.9;

    // the width of the view is unknow until we figure out how many items there are
    QRectF r(rect()); r.setHeight(vTileNum * (tileHeight + s_tileHistoryPadding));
    setGeometry(r);

    doLayoutTiles(rect(), hTileNum, tileWidth, vTileNum, tileHeight, s_tileHistoryPadding, s_tileHistoryPadding);
}

class BookmarkWidget : public TileBaseWidget {
    Q_OBJECT
public:
    BookmarkWidget(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void layoutTiles();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    QImage m_bookmarkIcon;
    QLinearGradient m_bckgGradient;
};

BookmarkWidget::BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileBaseWidget(parent, wFlags)
    , m_bookmarkIcon(":/data/icon/48x48/bookmarks_48.png")
{
    // setup bckg gradient
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(240, 240, 240)) << QGradientStop(0.30, QColor(114, 114, 114)) << QGradientStop(0.50, QColor(144, 144, 144)) << QGradientStop(0.70, QColor(134, 134, 134)) << QGradientStop(1.00, QColor(40, 40, 40));
    for (int j=0; j<stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);
}

void BookmarkWidget::layoutTiles()
{
    // add bookmark tiles
    QRectF r(rect());
    r.setLeft(r.left() + m_bookmarkIcon.width());

    int hTileNum = qMin(m_tileList.size(), (int)(r.width() / s_bookmarksTileWidth));
    int vTileNum = 1;

    doLayoutTiles(r, hTileNum, s_bookmarksTileWidth, vTileNum, r.height(), s_tileBookmarkPadding, s_tileBookmarkPadding);

    m_bckgGradient.setStart(rect().topLeft());
    m_bckgGradient.setFinalStop(rect().bottomLeft());
}

void BookmarkWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // FIXME: optimize it
    QRectF r(rect());
    r.adjust(10, -15, -10, 0);
    painter->setBrush(m_bckgGradient);
    painter->setPen(QColor(20, 20, 20));
    painter->drawRoundedRect(r, 12, 12);
    painter->drawImage(QPoint(10, rect().height() / 2 - m_bookmarkIcon.height() / 2), m_bookmarkIcon);
    TileBaseWidget::paint(painter, option, widget);
}

HomeView::HomeView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_bookmarkWidget(new BookmarkWidget(this, wFlags))
    , m_historyWidget(new HistoryWidget(this, wFlags))
    , m_pannableHistoryContainer(new PannableTileContainer(this, wFlags))
{
    m_bookmarkWidget->setZValue(1);
    m_historyWidget->setZValue(1);
    m_pannableHistoryContainer->setWidget(m_historyWidget);
    connect(m_bookmarkWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
    connect(m_historyWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

HomeView::~HomeView()
{
    delete m_bookmarkWidget;
    delete m_historyWidget;
    delete m_pannableHistoryContainer;
}

void HomeView::setGeometry(const QRectF& rect)
{
    QRectF currentRect(geometry());
    if (rect == currentRect)
        return;
    
    if (rect.width() != currentRect.width() || rect.height() != currentRect.height()) {
        // readjust subcontainers' sizes in case of size chage
        QRectF r(rect);
        // upper stripe for bookmarks
        r.setHeight(s_bookmarkStripeHeight);
        m_bookmarkWidget->setGeometry(r);

        // history comes next
        r.moveTop(r.bottom());
        r.setBottom(rect.bottom());
        m_pannableHistoryContainer->setGeometry(r);
        m_historyWidget->setGeometry(r);
    }
    TileSelectionViewBase::setGeometry(rect);
}

void HomeView::tileItemActivated(TileItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);

    if (!item || !item->urlItem())
        return;
    emit pageSelected(item->urlItem()->m_url);
}

void HomeView::tileItemClosed(TileItem* item)
{
    TileSelectionViewBase::tileItemClosed(item);
    // FIXME add bookmark content as well
    m_historyWidget->removeTile(*item);
}

void HomeView::tileItemEditingMode(TileItem* item)
{
    TileSelectionViewBase::tileItemEditingMode(item);
    // FIXME bookmarks, not yet.
    //m_bookmarkWidget->setEditMode(!m_bookmarkWidget->editMode());
    m_historyWidget->setEditMode(!m_historyWidget->editMode());
    update();
}

void HomeView::setupAnimation(bool in)
{
    // add both history and bookmark anim
    QPropertyAnimation* historyAnim = new QPropertyAnimation(m_historyWidget, "geometry");
    historyAnim->setDuration(800);
    QRectF r(m_historyWidget->geometry());

    if (in)
        r.moveTop(rect().top());
    QRectF hiddenHistory(r); hiddenHistory.moveTop(r.bottom());

    historyAnim->setStartValue(in ?  hiddenHistory : r);
    historyAnim->setEndValue(in ? r : hiddenHistory);

    historyAnim->setEasingCurve(in ? QEasingCurve::OutBack : QEasingCurve::InCubic);

    m_slideAnimationGroup->addAnimation(historyAnim);

    //
    QPropertyAnimation* bookmarkAnim = new QPropertyAnimation(m_bookmarkWidget, "geometry");
    bookmarkAnim->setDuration(800);
    r = m_bookmarkWidget->geometry();

    if (in)
        r.moveTop(rect().top());
    QRectF hiddenBookmark(r); hiddenBookmark.moveBottom(r.top());

    bookmarkAnim->setStartValue(in ?  hiddenBookmark : r);
    bookmarkAnim->setEndValue(in ? r : hiddenBookmark);

    bookmarkAnim->setEasingCurve(in ? QEasingCurve::OutBack : QEasingCurve::InCubic);

    m_slideAnimationGroup->addAnimation(bookmarkAnim);

    // hide the container
    if (in) {
        m_historyWidget->setGeometry(hiddenHistory);
        m_bookmarkWidget->setGeometry(hiddenBookmark);
    }
}

void HomeView::destroyViewItems()
{
    m_bookmarkWidget->removeAll();
    m_historyWidget->removeAll();
}

void HomeView::createViewItems()
{
    // recreate?
    destroyViewItems();
    //
    createBookmarkContent();
    createHistoryContent();
}

void HomeView::createBookmarkContent()
{
    UrlList* list = BookmarkStore::instance()->list();
    // FIXME: this 'All bookmarks' urlItem is being leaked
    TileItem* newTileItem = new TileItem(m_bookmarkWidget, *(new UrlItem(QUrl(), "All bookmarks", 0)), TileItem::Horizontal, false);
    m_bookmarkWidget->addTile(*newTileItem);
    connectItem(*newTileItem);
    //
    for (int i = 0; i < list->size(); ++i) {
        newTileItem = new TileItem(m_bookmarkWidget, *list->at(i), TileItem::Horizontal);
        m_bookmarkWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    m_bookmarkWidget->layoutTiles();
}

void HomeView::createHistoryContent()
{
    TileItem* newTileItem = 0;
    UrlList* list = HistoryStore::instance()->list();
    for (int i = 0; i < (s_maxHistoryTileNum - 1) && i < list->size(); ++i) {
        newTileItem = new TileItem(m_historyWidget, *list->at(i), TileItem::Vertical);
        m_historyWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    // FIXME: this 'All history'  is being leaked
    newTileItem = new TileItem(m_historyWidget, *(new UrlItem(QUrl(), "All history", 0)), TileItem::Vertical, false);
    m_historyWidget->addTile(*newTileItem);
    connectItem(*newTileItem);

    m_historyWidget->layoutTiles();
}

#include "HomeView.moc"
