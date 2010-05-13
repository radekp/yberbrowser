#include "PannableTileContainer.h"
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QGraphicsScene>
#include <QCoreApplication>
//temp
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
