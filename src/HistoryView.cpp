#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>

#include "HistoryView.h"
#include "HistoryItem.h"
#include "UrlStore.h"
#include "ApplicationWindow.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#if USE_DUI
#include <DuiScene>
#endif

HistoryView::HistoryView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_bckg(new QGraphicsRectItem(rect(), this))
    , m_tileContainer(new QGraphicsWidget(this))
    , m_slideAnim(new QPropertyAnimation(m_tileContainer, "geometry"))
    , m_active(false)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);

    m_bckg->setPen(QPen(QBrush(QColor(10, 10, 10)), 3));
    m_bckg->setBrush(QColor(20, 20, 20));

    connect(m_slideAnim, SIGNAL(finished()), this, SLOT(animFinished()));
}

HistoryView::~HistoryView()
{
    destroyHistoryTiles();
    delete m_slideAnim;
    delete m_tileContainer;
    delete m_bckg;
}

void HistoryView::setGeometry(const QRectF& rect)
{
    if (rect == geometry())
        return;

    QGraphicsWidget::setGeometry(rect);

    m_bckg->setRect(rect);
    m_tileContainer->setGeometry(rect);

    if (m_active)
        createHistoryTiles();
}

void HistoryView::createHistoryTiles()
{
    int width = rect().width();

    int minWidth = 170;
    int maxWidth = 200;

    if (width < minWidth)
        return;

    int hTileNum = 3;
    int tileWidth = width / hTileNum;

    while (tileWidth < minWidth && hTileNum > 0)
        tileWidth = width / --hTileNum;

    tileWidth = qMin(tileWidth, maxWidth);

    // keep height proposional
    int height = rect().height();
    int vTileNum = 3;
    int tileHeight = height / vTileNum;
    int minHeight = tileWidth / 1.20;
    int maxHeight = tileWidth / 1.10;

    if (height < minHeight)
        return;

    while (tileHeight < minHeight && vTileNum > 0)
        tileHeight = height / --vTileNum;

    tileHeight = qMin(tileHeight, maxHeight);

    UrlList& list = UrlStore::instance()->list();
    // move tiles to the middle
    int y = (height - (vTileNum * tileHeight)) / 2;
    for (int i = 0; i < vTileNum; ++i) {
        int x = (width - (hTileNum * tileWidth)) / 2;
        for (int j = 0; j < hTileNum; ++j) {
            // get the corresponding history entry
            int itemIndex = j + (i*hTileNum);
            UrlItem* urlItem = 0;
            if (itemIndex < list.size())
                urlItem = list.at(itemIndex);

            // reposition or create a new (expanding)
            HistoryItem* item;
            if (itemIndex < m_historyList.size())
                item = m_historyList.at(itemIndex);
            else {
                item = new HistoryItem(m_tileContainer, urlItem);
                connect(item, SIGNAL(itemActivated(UrlItem*)), this, SLOT(historyItemActivated(UrlItem*)));
                m_historyList.append(item);
            }
            item->setGeometry(QRectF(x + 20, y + 20, tileWidth - (2*20), tileHeight  - (2*20)));
            x+=(tileWidth);
        }
        y+=(tileHeight);
    }
    // remove items that fall off (shrinking)
    int tilesNum = vTileNum * hTileNum;
    if (tilesNum < m_historyList.size()) {
        for (int i = m_historyList.size() - 1; i >= tilesNum; --i)
            delete m_historyList.takeAt(i);
    }
}

void HistoryView::destroyHistoryTiles()
{
    for (int i = m_historyList.size() - 1; i >= 0; --i)
        delete m_historyList.takeAt(i);
}

void HistoryView::animFinished()
{
    // destroy thumbs when animation finished (outbound)
    if (!m_active) {
        emit disappeared();
        destroyHistoryTiles();
    } else {
        // set transparency and dropshadow only when anim is finished to avoid rendering artifacts
        m_bckg->setOpacity(0.8);
        for (int i = 0; i < m_historyList.size(); ++i)
            m_historyList.at(i)->addDropshadow();
    }
}

void HistoryView::historyItemActivated(UrlItem* item)
{
    disappear();
    if (item)
        emit urlSelected(item->m_url);
}

void HistoryView::startAnimation(bool in)
{
    // ongoing?
    m_slideAnim->stop();
    m_slideAnim->setDuration(800);

    QRectF r(in ? rect() : m_tileContainer->geometry());
    QRectF hidden(r); hidden.moveTop(r.bottom());

    m_slideAnim->setStartValue(in ?  hidden : r);
    m_slideAnim->setEndValue(in ? r : hidden);

    m_slideAnim->setEasingCurve(in ? QEasingCurve::OutBack : QEasingCurve::InCubic);
    m_slideAnim->start(QAbstractAnimation::KeepWhenStopped);
    
    // hide the container
    if (in)
        m_tileContainer->setGeometry(hidden);
}

void HistoryView::appear(ApplicationWindow *window)
{
    // FIXME: how to test if historyview is already in correct view?
    if (!scene()) {
        window->scene()->addItem(this);
        setZValue(100);
    }
    scene()->setActiveWindow(this);

    m_active = true;
    createHistoryTiles();
    startAnimation(m_active);
}

void HistoryView::disappear()
{
    m_active = false;
    startAnimation(m_active);
}

