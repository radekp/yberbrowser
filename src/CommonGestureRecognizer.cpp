#include <QtGui>

#include "CommonGestureRecognizer.h"
#include "EventHelpers.h"

static const int s_waitForClickTimeoutMS = 200;

// time between mouse release that was part of pan and
// double tap that can happen
static const int s_doubleClickFilterDurationMS = 300;

static const int s_minTimeHoldForClick = 100;

/* TODO

    - This should go away

    - At max, this should synthetize TouchEvents

    - The event coords are not welldefined, since the events might
      sometimes come from the container (WebViewportItem), sometimes
      from the childs (Web View)

    - This should do mouse click filtering, as it is very unreliable
      on device.
 */

CommonGestureRecognizer::CommonGestureRecognizer(CommonGestureConsumer* consumer)
    : m_consumer(consumer)
    , m_delayedPressEvent(0)
    , m_delayedReleaseEvent(0)
    , m_pressDelay(300)

{
    reset();
}

CommonGestureRecognizer::~CommonGestureRecognizer()
{
    clearDelayedPress();
}

bool CommonGestureRecognizer::filterMouseEvent(QGraphicsSceneMouseEvent *event)
{
    // these events will be sent by this class, don't filter these
    if (m_delayedPressEvent == event || m_delayedReleaseEvent == event)
        return false;

    bool accepted = false;

    switch(event->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
        clearDelayedPress();
        accepted = mouseDoubleClickEvent(event);
        break;
    case QEvent::GraphicsSceneMouseMove:
        accepted = mouseMoveEvent(event);
        break;
    case QEvent::GraphicsSceneMousePress:
        accepted = mousePressEvent(event);
        break;

    case QEvent::GraphicsSceneMouseRelease:
        accepted = mouseReleaseEvent(event);
        break;
    default:
        break;
    }

    return accepted;
}

void CommonGestureRecognizer::capturePressOrRelease(QGraphicsSceneMouseEvent *event, bool wasRelease)
{
    if (m_pressDelay <= 0)
        return;

    QGraphicsSceneMouseEvent* delayedEvent = new QGraphicsSceneMouseEvent(event->type());
    copyMouseEvent(event, delayedEvent);
    delayedEvent->setAccepted(false);

    if (wasRelease) {
        m_delayedReleaseEvent = delayedEvent;
        // mouse press is more reliable than mouse release
        // we must send release with same coords as press
        copyMouseEventPositions(m_delayedPressEvent, m_delayedReleaseEvent);
        m_delayedPressTimer.start(m_pressDelay, this);
    } else {
        m_delayedPressEvent = delayedEvent;
        m_delayedPressMoment.start();
    }
}

void CommonGestureRecognizer::clearDelayedPress()
{
    if (m_delayedPressEvent) {
        m_delayedPressTimer.stop();
        delete m_delayedPressEvent;
        m_delayedPressEvent = 0;
        delete m_delayedReleaseEvent;
        m_delayedReleaseEvent = 0;
    }
}

void CommonGestureRecognizer::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_delayedPressTimer.timerId()) {
        m_delayedPressTimer.stop();
        m_consumer->tapGesture(m_delayedPressEvent, m_delayedReleaseEvent);
        clearDelayedPress();
    }
}

bool CommonGestureRecognizer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos() << " filter: " <<m_doubleClickFilter.elapsed();
#endif

    if (m_doubleClickFilter.elapsed() > s_doubleClickFilterDurationMS)
        m_consumer->doubleTapGesture(event);
    else
        mousePressEvent(event);
    return true;
}


bool CommonGestureRecognizer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos();
#endif

    // long tap causes left button click, don't send it further
    if (event->button() != Qt::LeftButton)
        return true;

    if (!m_delayedPressEvent)
        capturePressOrRelease(event);

    if (m_consumer)
        m_consumer->touchGestureBegin(event->screenPos());

    return true;
}

bool CommonGestureRecognizer::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos();
#endif

    if (event->button() != Qt::LeftButton)
        return true;

    if (m_consumer)
        m_consumer->touchGestureEnd();

    if (m_delayedReleaseEvent) {
        // sometimes double click is lost if small mouse move occurs
        // inbetween
        QGraphicsSceneMouseEvent dblClickEvent(QEvent::GraphicsSceneMouseDoubleClick);
        copyMouseEvent(m_delayedPressEvent, &dblClickEvent);
        mouseDoubleClickEvent(&dblClickEvent);
    } else if (m_delayedPressEvent && !m_delayedReleaseEvent) {
        if (m_delayedPressMoment.elapsed() > s_minTimeHoldForClick)
            capturePressOrRelease(event, true);
        else
            clearDelayedPress();
    }
    return true;
}

bool CommonGestureRecognizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
#if defined(ENABLE_EVENT_DEBUG)
    qDebug() << __PRETTY_FUNCTION__ << event << event->screenPos();
#endif
    Q_UNUSED(event);
    // filter anyway since we don't want excess link hovers when panning.
    return true;
}

void CommonGestureRecognizer::reset()
{
    clearDelayedPress();
}

