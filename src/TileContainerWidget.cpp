#include "TileContainerWidget.h"
#include <QApplication>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

PannableTileContainer::PannableTileContainer(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : PannableViewport(parent, wFlags)
    , m_recognizer(this)
    , m_selfSentEvent(0)
{
    m_recognizer.reset();
}

PannableTileContainer::~PannableTileContainer()
{
}

bool PannableTileContainer::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    if (e == m_selfSentEvent)
        return false;
    /* Apply super class event filter. This will capture mouse
    move for panning.  it will return true when applies panning
    but false until pan events are recognized
    */
    bool doFilter = PannableViewport::sceneEventFilter(i, e);

    if (!isVisible())
        return doFilter;

    switch (e->type()) {
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick:
        m_recognizer.filterMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        doFilter = true;
        break;
    default:
        break;
    }

    return doFilter;
}

void PannableTileContainer::cancelLeftMouseButtonPress(const QPoint&)
{
    // don't send the mouse press event after this callback.
    // QAbstractCKineticScroller started panning
    m_recognizer.clearDelayedPress();
}

void PannableTileContainer::mousePressEventFromChild(QGraphicsSceneMouseEvent* event)
{
    forwardEvent(event);
}

void PannableTileContainer::mouseReleaseEventFromChild(QGraphicsSceneMouseEvent* event)
{
    forwardEvent(event);
}

void  PannableTileContainer::mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent* event)
{
    forwardEvent(event);
}

void PannableTileContainer::forwardEvent(QGraphicsSceneMouseEvent* event)
{
    m_selfSentEvent = event;
    QApplication::sendEvent(scene(), event);
    m_selfSentEvent = 0;
}

TileBaseWidget::TileBaseWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_slideAnimationGroup(0)
    , m_editMode(false)
{
}

TileBaseWidget::~TileBaseWidget()
{
    removeAll();
    delete m_slideAnimationGroup;
}

void TileBaseWidget::doLayoutTiles(const QRectF& rect, int hTileNum, int tileWidth, int vTileNum, int tileHeight, int paddingX, int paddingY)
{
    if (!m_tileList.size())
        return;
    int width = rect.width();
    int y = rect.top() + paddingY;
    for (int i = 0; i < vTileNum; ++i) {
        // move tiles to the middle
        int x = qMax(0, (int)(rect.left() + (width - (hTileNum * tileWidth)) / 2));
        for (int j = 0; j < hTileNum; ++j) {
            // get the corresponding url entry
            int itemIndex = j + (i*hTileNum);
            if (itemIndex >= m_tileList.size())
                continue;
            // padding
            m_tileList.at(itemIndex)->setGeometry(QRectF(x + paddingX, y + paddingY, tileWidth - (2*paddingX), tileHeight  - (2*paddingY)));
            x+=(tileWidth);
        }
        y+=(tileHeight);
    }
    // leftovers are hidden
    for (int i = vTileNum*hTileNum; i < m_tileList.size(); ++i)
        m_tileList.at(i)->hide();
}

void TileBaseWidget::addTile(TileItem& newItem)
{
    m_tileList.append(&newItem);
    newItem.setEditMode(m_editMode);
}

void TileBaseWidget::removeTile(TileItem& removed)
{
    if (!m_slideAnimationGroup)
        m_slideAnimationGroup = new QParallelAnimationGroup();

    m_slideAnimationGroup->clear();
    for (int i = 0; i < m_tileList.size(); ++i) {
        if (m_tileList.at(i) == &removed) {
            for (int j = m_tileList.size() - 1; j > i; --j)
                addMoveAnimation(*m_tileList.at(j), (j - i) * 100, m_tileList[j]->rect(), m_tileList[j-1]->rect());
            delete m_tileList.takeAt(i);
            break;
        }
    }
    m_slideAnimationGroup->start(QAbstractAnimation::KeepWhenStopped);
}

void TileBaseWidget::removeAll()
{
    for (int i = m_tileList.size() - 1; i >= 0; --i)
        delete m_tileList.takeAt(i);
}

void TileBaseWidget::setEditMode(bool on) 
{ 
    m_editMode = on;
    for (int i = 0; i < m_tileList.size(); ++i)
        m_tileList.at(i)->setEditMode(on);
}

void TileBaseWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    emit closeWidget();
}

void TileBaseWidget::addMoveAnimation(TileItem& item, int delay, const QRectF& oldPos, const QRectF& newPos)
{
    // animate all the way down to the current window
    QPropertyAnimation* moveAnim = new QPropertyAnimation(&item, "geometry");
    moveAnim->setDuration(500 + delay);

    moveAnim->setStartValue(oldPos);
    moveAnim->setEndValue(newPos);

    moveAnim->setEasingCurve(QEasingCurve::OutBack);
    m_slideAnimationGroup->addAnimation(moveAnim);
}


