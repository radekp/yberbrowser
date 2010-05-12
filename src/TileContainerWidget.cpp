#include "TileContainerWidget.h"
#include <QApplication>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPainter>
#include "HomeView.h"

PannableTileContainer::PannableTileContainer(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : PannableViewport(parent, wFlags)
    , m_recognizer(this)
    , m_selfSentEvent(0)
    , m_homeView(0)
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
        if (m_homeView)
            doFilter = m_homeView->recognizeFlick(static_cast<QGraphicsSceneMouseEvent *>(e));
        if (!doFilter)
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

TileBaseWidget::TileBaseWidget(const QString& title, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_slideAnimationGroup(0)
    , m_title(title)
    , m_editMode(false)
{
}

TileBaseWidget::~TileBaseWidget()
{
    removeAll();
    delete m_slideAnimationGroup;
}

QSize TileBaseWidget::doLayoutTiles(const QRectF& rect_, int hTileNum, int vTileNum, int marginX, int marginY, bool fixed)
{
    if (!m_tileList.size())
        return QSize(0, 0) ;

    int width = rect_.width() - (hTileNum + 1)*marginX;
    int height = rect_.height() - (vTileNum + 1)*marginY;

    int tileWidth = width / hTileNum;
    int tileHeight = height / vTileNum; 

    int tileCount = fixed ? qMin(hTileNum * vTileNum, m_tileList.size()) : m_tileList.size();
    int y = rect_.top() + s_viewMargin - tileHeight;
    int x = rect_.left() + marginX;
    int i = 0;
    for (; i < tileCount; ++i) {
        if (i%hTileNum == 0) {
            y+=(tileHeight + marginY);
            x = rect_.left() + marginX;
        }
        m_tileList.at(i)->setRect(QRectF(x, y, tileWidth, tileHeight));
        x+=(tileWidth + marginX);
    }

    if (fixed) {
        // leftovers are hidden, lined up after the last item
        for (int j = i; j < m_tileList.size(); ++j) {
            m_tileList.at(j)->setRect(QRectF(x, y, tileWidth, tileHeight));
            x+=(tileWidth + marginX);
            m_tileList.at(j)->hide();
        }
    }
    return QSize(tileWidth * hTileNum, y);
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
                addMoveAnimation(*m_tileList.at(j), (j - i) * 50, m_tileList[j]->rect().topLeft(), m_tileList[j-1]->rect().topLeft());
            delete m_tileList.takeAt(i);
            break;
        }
    }
    // any hidden item to appear?
// do ishidden()
//    if (m_tileList.size() >= m_maxTileCount)
//        m_tileList.at(m_maxTileCount - 1)->show();
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

void TileBaseWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QGraphicsWidget::paint(painter, option, widget);
    painter->setFont(QFont("Times", 14));
    painter->setPen(Qt::white);
    QRectF r(rect());
    r.setHeight(s_viewMargin);
    painter->drawText(r, Qt::AlignCenter, m_title);
}

void TileBaseWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    emit closeWidget();
}

void TileBaseWidget::addMoveAnimation(TileItem& item, int delay, const QPointF& oldPos, const QPointF& newPos)
{
    // animate all the way down to the current window
    QPropertyAnimation* moveAnim = new QPropertyAnimation(&item, "tilePos");
    moveAnim->setDuration(500 + delay);

    moveAnim->setStartValue(oldPos);
    moveAnim->setEndValue(newPos);

    moveAnim->setEasingCurve(QEasingCurve::OutBack);
    m_slideAnimationGroup->addAnimation(moveAnim);
}


