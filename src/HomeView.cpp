#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>

#include "HomeView.h"
#include "TileItem.h"
#include "UrlStore.h"
#include "ApplicationWindow.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#if USE_DUI
#include <DuiScene>
#endif

namespace {
const int s_minTileWidth = 170;
const int s_maxTileWidth = 200;
const int s_defaultTileNumH = 3;
const int s_defaultTileNumV = 6;
const int s_tilePadding = 20;
const int s_bookmarkStripeHeight = 60;
const int s_bookmarksTileWidth = 120;
const QColor s_TitleTextColor(0xFA, 0xFA, 0xFA);
}

HomeView::HomeView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : PannableViewport(parent, wFlags)
    , m_bckg(new QGraphicsRectItem(rect(), this))
    , m_homeContainer(new HomeContainer(this, wFlags))
    , m_slideAnim(new QPropertyAnimation(m_homeContainer, "geometry"))
    , m_active(false)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);

    m_bckg->setPen(QPen(QBrush(QColor(10, 10, 10)), 3));
    m_bckg->setBrush(QColor(20, 20, 20));

    m_bckg->setZValue(0);
    m_homeContainer->setZValue(1);
    connect(m_slideAnim, SIGNAL(finished()), this, SLOT(animFinished()));

    setWidget(m_homeContainer);
}

HomeView::~HomeView()
{
    delete m_slideAnim;
    delete m_bckg;
    delete m_homeContainer;
}

void HomeView::setGeometry(const QRectF& rect)
{
    if (rect == geometry())
        return;
    QGraphicsWidget::setGeometry(rect);
    m_bckg->setRect(rect);
    // home container is twice the height
    QRectF r(rect);
    r.setHeight(2*r.height());
    m_homeContainer->setGeometry(r);
}

void HomeView::appear(ApplicationWindow *window)
{
    // FIXME: how to test if view is already in correct view?
    if (!scene()) {
        window->scene()->addItem(this);
        setZValue(100);
    }
    scene()->setActiveWindow(this);

    m_active = true;
    m_homeContainer->createViewItems();
    startAnimation(m_active);
}

void HomeView::disappear()
{
    m_active = false;
    startAnimation(m_active);
}

void HomeView::animFinished()
{
    // destroy thumbs when animation finished (outbound)
    if (m_active) {
        // set transparency
        m_bckg->setOpacity(0.8);
        emit appeared();
    } else {
        m_homeContainer->destroyViewItems();
        emit disappeared();
    }
}

void HomeView::tileItemActivated(UrlItem* item)
{
    disappear();
    if (item)
        emit urlSelected(item->m_url);
}

void HomeView::startAnimation(bool in)
{
    // ongoing?
    m_slideAnim->stop();
    m_slideAnim->setDuration(800);
    QRectF r(m_homeContainer->geometry());

    if (in)
        r.moveTop(rect().top());
    QRectF hidden(r); hidden.moveTop(r.bottom());

    m_slideAnim->setStartValue(in ?  hidden : r);
    m_slideAnim->setEndValue(in ? r : hidden);

    m_slideAnim->setEasingCurve(in ? QEasingCurve::OutBack : QEasingCurve::InCubic);
    m_slideAnim->start(QAbstractAnimation::KeepWhenStopped);
    
    // hide the container
    if (in)
        m_homeContainer->setGeometry(hidden);
}

//-------------------------

HomeContainer::HomeContainer(HomeView* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_historyContainer(new HistoryContainer(this, wFlags))
    , m_bookmarkContainer(new BookmarkContainer(this, wFlags))
{
    connect(parent, SIGNAL(appeared()), m_historyContainer, SLOT(appeared()));
    connect(parent, SIGNAL(appeared()), m_bookmarkContainer, SLOT(appeared()));
}

HomeContainer::~HomeContainer()
{
    delete m_historyContainer;
    delete m_bookmarkContainer;
}

void HomeContainer::setGeometry(const QRectF& rect)
{
    QRectF currentRect(geometry());
    if (rect == currentRect)
        return;
    QGraphicsWidget::setGeometry(rect);
    
    // readjust subcontainers' sizes in case of size chage
    if (rect.width() != currentRect.width() || rect.height() != currentRect.height()) {
        QRectF r(rect);
        // upper stripe for bookmarks
        r.setHeight(s_bookmarkStripeHeight);
        m_bookmarkContainer->setGeometry(r);
        // history comes next
        r.moveTop(r.bottom());
        r.setHeight(rect.height() - r.height());
        m_historyContainer->setGeometry(r);
        // reconstuct tiles
        destroyViewItems();
        createViewItems();
    }
}

void HomeContainer::createViewItems()
{
    m_historyContainer->createTiles();
    m_bookmarkContainer->createTiles();
}

void HomeContainer::destroyViewItems()
{
    m_historyContainer->destroyTiles();
    m_bookmarkContainer->destroyTiles();
}


TileContainer::TileContainer(UrlList& urlList, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_urlList(&urlList)
{
}

TileContainer::~TileContainer()
{
    destroyTiles();
}

void TileContainer::addTiles(int hTileNum, int tileWidth, int vTileNum, int tileHeight, int padding, bool textOnly)
{
    QGraphicsWidget* parentView = parentWidget()->parentWidget();
    if (m_tileList.size())
        return;

    int width = rect().width();
    int y = padding;
    for (int i = 0; i < vTileNum; ++i) {
        // move tiles to the middle
        int x = (width - (hTileNum * tileWidth)) / 2;
        for (int j = 0; j < hTileNum; ++j) {
            // get the corresponding url entry
            int itemIndex = j + (i*hTileNum);
            if (itemIndex >= m_urlList->size())
                continue;

            // create new tile item
            TileItem* item = new TileItem(this, m_urlList->at(itemIndex), textOnly);
            connect(item, SIGNAL(itemActivated(UrlItem*)), parentView, SLOT(tileItemActivated(UrlItem*)));
            // padding
            item->setGeometry(QRectF(x + padding, y + padding, tileWidth - (2*padding), tileHeight  - (2*padding)));
            m_tileList.append(item);
            x+=(tileWidth);
        }
        y+=(tileHeight);
    }
}

void TileContainer::destroyTiles()
{
    for (int i = m_tileList.size() - 1; i >= 0; --i)
        delete m_tileList.takeAt(i);
}

void TileContainer::appeared()
{
    // add  dropshadow only when anim is finished to avoid rendering artifacts
    for (int i = 0; i < m_tileList.size(); ++i)
        m_tileList.at(i)->addDropshadow();
}

HistoryContainer::HistoryContainer(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileContainer(UrlStore::instance()->list(), parent, wFlags)
{
}

void HistoryContainer::createTiles()
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

    addTiles(hTileNum, tileWidth, vTileNum, tileHeight, s_tilePadding, false);
}

BookmarkContainer::BookmarkContainer(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileContainer(m_list, parent, wFlags)
{
    // FIXME : hardcoded static items in the bookmarks view until bookmark management is added.
    m_list.append(new UrlItem(QUrl("http://cnn.com/"), "CNN.com - Breaking News, U.S., World, Weather, Entertainment &amp; Video News"));
    m_list.append(new UrlItem(QUrl("http://news.bbc.co.uk/"), "BBC NEWS | News Front Page"));
    m_list.append(new UrlItem(QUrl("http://news.google.com/"), "Google News"));
    m_list.append(new UrlItem(QUrl("http://nokia.com/"), "Nokia - Nokia on the Web"));
    m_list.append(new UrlItem(QUrl("http://qt.nokia.com/"), "Qt - A cross-platform application and UI framework"));
    m_list.append(new UrlItem(QUrl("http://ovi.com/"), "Ovi by Nokia"));
    m_list.append(new UrlItem(QUrl("http://nytimes.com/"), "The New York Times - Breaking News, World News Multimedia"));
    m_list.append(new UrlItem(QUrl("http://google.com/"), "Google"));
}

void BookmarkContainer::createTiles()
{
    addTiles(rect().width() / s_bookmarksTileWidth, s_bookmarksTileWidth, 1, rect().height(), 5,  true);
}

