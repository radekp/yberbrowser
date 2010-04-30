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
const int s_tilePadding = 20;
const int s_bookmarkStripeHeight = 70;
const int s_bookmarksTileWidth = 120;
const QColor s_TitleTextColor(0xFA, 0xFA, 0xFA);
}

class HistoryWidget : public TileBaseWidget {
    Q_OBJECT
public:
    HistoryWidget(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void setupWidgetContent();
};

class BookmarkWidget : public TileBaseWidget {
    Q_OBJECT
public:
    BookmarkWidget(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void setupWidgetContent();

private:
    QGraphicsRectItem* m_bckg;
    QLinearGradient m_bckgGradient;
};

HistoryWidget::HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileBaseWidget(HistoryStore::instance()->list(), parent, wFlags)
{
}

void HistoryWidget::setupWidgetContent()
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
    int height = rect().height();
    int vTileNum = s_defaultTileNumV;
    int tileHeight = height / vTileNum;
    int minHeight = tileWidth / 1.20;
    int maxHeight = tileWidth / 1.10;

    if (height < minHeight)
        return;
    
    // calculate tile height
    while (tileHeight < minHeight && vTileNum > 0)
        tileHeight = height / --vTileNum;

    tileHeight = qMin(tileHeight, maxHeight);

    addTiles(rect(), hTileNum, tileWidth, vTileNum, tileHeight, s_tilePadding, s_tilePadding, TileItem::Vertical);
}

BookmarkWidget::BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileBaseWidget(BookmarkStore::instance()->list(), parent, wFlags)
{
    // setup bck gradient
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(240, 240, 240)) << QGradientStop(0.30, QColor(114, 114, 114)) << QGradientStop(0.50, QColor(144, 144, 144)) << QGradientStop(0.70, QColor(134, 134, 134)) << QGradientStop(1.00, QColor(40, 40, 40));
    for (int j=0; j<stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);
}

void BookmarkWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // FIXME: optimize it
    QRectF r(rect());
    r.adjust(10, -15, -10, 0);
    painter->setBrush(m_bckgGradient);
    painter->setPen(QColor(20, 20, 20));
    painter->drawRoundedRect(r, 12, 12);
    QImage bm(":/data/icon/48x48/bookmarks_48.png");
    painter->drawImage(QPoint(10, rect().height() / 2 - bm.size().height() / 2), bm);
    TileBaseWidget::paint(painter, option, widget);
}

void BookmarkWidget::setupWidgetContent()
{
    // add bookmark tiles
    QRectF r(rect());
    m_bckgGradient.setStart(r.topLeft());
    m_bckgGradient.setFinalStop(r.bottomLeft());
    r.adjust(50, 10, -15, 0);
    addTiles(r, qMin(m_urlList->size(), (int)(r.width() / s_bookmarksTileWidth)), s_bookmarksTileWidth, 1, r.height() - 10, 5, 0, TileItem::Horizontal);
}

HomeView::HomeView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_historyWidget(new HistoryWidget(this, wFlags))
    , m_bookmarkWidget(new BookmarkWidget(this, wFlags))
    , m_pannableHistoryContainer(new PannableTileContainer(this, wFlags))
{
    m_historyWidget->setZValue(1);
    m_bookmarkWidget->setZValue(1);
    m_pannableHistoryContainer->setWidget(m_historyWidget);
    connect(m_historyWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
    connect(m_bookmarkWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

HomeView::~HomeView()
{
    delete m_historyWidget;
    delete m_bookmarkWidget;
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
        // and the panned widget is twice as the view
        r.setHeight(2 * rect.height());
        m_historyWidget->setGeometry(r);
    }
    TileSelectionViewBase::setGeometry(rect);
}

void HomeView::tileItemActivated(UrlItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);

    if (!item)
        return;
    emit urlSelected(item->m_url);
}

void HomeView::tileItemEdited(UrlItem* item)
{
    TileSelectionViewBase::tileItemEdited(item);

    if (!item)
        return;
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
    m_historyWidget->destroyWidgetContent();
    m_bookmarkWidget->destroyWidgetContent();
}

void HomeView::createViewItems()
{
    m_historyWidget->setupWidgetContent();
    m_bookmarkWidget->setupWidgetContent();
}

#include "HomeView.moc"
