#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>

#include "HistoryViewportItem.h"
#include "MainView.h"
#include "MainWindow.h"
#include "HistoryItem.h"
#include "UrlStore.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class TileBackground : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    TileBackground(const QRectF& rect, QGraphicsItem* parent) : QGraphicsRectItem(rect, parent) {}
};

HistoryViewportItem::HistoryViewportItem(MainView& view, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_view(&view)
    , m_bckg(0)
    , m_animGroup(new QParallelAnimationGroup)
    , m_active(false)
    // fixme: remove m_ongoing, it is hack anyway
    , m_ongoing(false)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    
    connect(m_animGroup, SIGNAL(finished()), this, SLOT(animFinished()));
}

HistoryViewportItem::~HistoryViewportItem()
{
    delete m_animGroup;
    destroyHistoryTiles();
}

void HistoryViewportItem::setGeometry(const QRectF& rect)
{
    if (rect == geometry())    
        return;

    QGraphicsWidget::setGeometry(rect);
    if (m_active)
        createHistoryTiles();
}

void HistoryViewportItem::toggleHistory()
{
    m_active = !m_active;
    if (m_active)
        createHistoryTiles();
    startAnimation(m_active);
}

void HistoryViewportItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    // turn history view off
    if (m_active)
        m_view->toggleHistory();
}

void HistoryViewportItem::createHistoryTiles()
{
    // background first
    if (!m_bckg) {
        m_bckg = new TileBackground(rect(), this);
        m_bckg->setPen(QPen(QBrush(QColor(10, 10, 10)), 3));
        m_bckg->setBrush(QColor(20, 20, 20));
    }
    else 
        m_bckg->setRect(rect());

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
                item = new HistoryItem(this, urlItem);  
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

void HistoryViewportItem::destroyHistoryTiles()
{
    for (int i = m_historyList.size() - 1; i >= 0; --i)
        delete m_historyList.takeAt(i);
    delete m_bckg;
    m_bckg = 0;
}

void HistoryViewportItem::animFinished()
{
    m_ongoing = false;
    // destroy thumbs when animation finished (outbound)
    if (!m_active) {
        emit hideHistory();
        destroyHistoryTiles();
    }
}

void HistoryViewportItem::historyItemActivated(UrlItem* item)
{
    toggleHistory();
    if (item)
        m_view->mainWindow()->load(item->m_url.toString());
}

void HistoryViewportItem::startAnimation(bool in)
{
    if (!m_historyList.size()) {
        // keep the state sane, even where there is no items
        if (!m_active)
            emit hideHistory();
        return;
    }

    // get the topmost item and calculate the slide distance accordingly
    unsigned dist = rect().height() - m_historyList.at(0)->rect().y();
    QPoint startPos(0, in ? dist : 0);
    QPoint endPos(0, in ? 0 : dist);
    QEasingCurve::Type curve = in ? QEasingCurve::OutBack : QEasingCurve::OutQuint;

    // ongoing animation?
    m_animGroup->stop();
    // fixme big hack just beacuse i cant manage states
    if (m_ongoing && !m_active) {
        m_active = true;
        m_animGroup->clear();
        m_active = false;
    } else 
        m_animGroup->clear();

    for (int i = 0; i < m_historyList.size(); ++i) {
        QPropertyAnimation* animation = new QPropertyAnimation(m_historyList.at(i), "pos");
        animation->setDuration(800);

        animation->setStartValue(startPos);
        animation->setEndValue(endPos);

        animation->setEasingCurve(curve);
        
        m_animGroup->addAnimation(animation);
    }
    
    QPropertyAnimation* animation = new QPropertyAnimation(m_bckg, "opacity");
    animation->setDuration(800);

    animation->setStartValue(in ? 0.0 : 0.8);
    animation->setEndValue(in ? 0.8 : 0.0);

    animation->setEasingCurve(QEasingCurve::InQuad);
    
    m_animGroup->addAnimation(animation);

    m_ongoing = true;
    m_animGroup->start();
}

#include "HistoryViewportItem.moc"
