#include <QtGui>

#include "CommonGestureRecognizer.h"

static const int s_waitForClickTimeoutMS = 200;
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
        if (m_delayedPressEvent && !m_delayedReleaseEvent && accepted)
            captureDelayedPress(event, true);
        break;
    default:
        break;
    }

    return accepted;
}

void CommonGestureRecognizer::captureDelayedPress(QGraphicsSceneMouseEvent *event, bool wasRelease)
{
    Q_ASSERT(!m_delayedPressEvent && !wasRelease);
    Q_ASSERT(m_delayedPressEvent && !m_delayedReleaseEvent && wasRelease);

    if (m_pressDelay <= 0)
        return;

    QGraphicsSceneMouseEvent* delayedEvent = new QGraphicsSceneMouseEvent(event->type());
    delayedEvent->setAccepted(false);
    for (int i = 0x1; i <= 0x10; i <<= 1) {
        if (event->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            delayedEvent->setButtonDownPos(button, event->buttonDownPos(button));
            delayedEvent->setButtonDownScenePos(button, event->buttonDownScenePos(button));
            delayedEvent->setButtonDownScreenPos(button, event->buttonDownScreenPos(button));
        }
    }
    delayedEvent->setButtons(event->buttons());
    delayedEvent->setButton(event->button());
    delayedEvent->setPos(event->pos());
    delayedEvent->setScenePos(event->scenePos());
    delayedEvent->setScreenPos(event->screenPos());
    delayedEvent->setLastPos(event->lastPos());
    delayedEvent->setLastScenePos(event->lastScenePos());
    delayedEvent->setLastScreenPos(event->lastScreenPos());
    delayedEvent->setModifiers(event->modifiers());
    delayedEvent->setWidget(event->widget());

    if (wasRelease) {
        m_delayedReleaseEvent = delayedEvent;
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
    m_consumer->doubleTapGesture(event);
    return true;
}


bool CommonGestureRecognizer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return false;

    if (!m_delayedPressEvent)
        captureDelayedPress(event);

    return true;
}

bool CommonGestureRecognizer::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return false;

    if (m_consumer->isPanning())
        m_consumer->stopPanGesture();

    return true;
}

bool CommonGestureRecognizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    bool isPanning = m_consumer->isPanning();

    if (!isPanning) {
        // if the button is still pressed
        if (m_delayedPressEvent && !m_delayedReleaseEvent) {
            QPoint d = m_delayedPressEvent->screenPos() - event->screenPos();
            if (qAbs(d.y()) > QApplication::startDragDistance()
                || qAbs(d.x()) > QApplication::startDragDistance()
                || m_delayedPressMoment.elapsed() > s_waitForClickTimeoutMS) {
                m_consumer->startPanGesture();
                isPanning = m_consumer->isPanning();

                // to avoid initial warping, don't use m_delayedPressEvent->screenPos()
                m_dragStartPos = event->screenPos();
                clearDelayedPress();
            }
        }
    }

    if (isPanning) {
        clearDelayedPress();
        m_consumer->panBy(event->screenPos() - m_dragStartPos);
        m_dragStartPos = event->screenPos();
        return true;
    }

    return false;
}

void CommonGestureRecognizer::reset()
{
    m_dragStartPos = QPoint();
    clearDelayedPress();
}

