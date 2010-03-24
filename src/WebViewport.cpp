#include <QApplication>
#include <QGraphicsScene>
#include "WebViewport.h"
#include "WebViewportItem.h"
#include "EventHelpers.h"

namespace {
const int s_zoomAnimDurationMS = 300;

const float s_zoomScaleWheelStep = .2;
const int s_doubleClickWaitTimeout = 100;
}

/*!
  \class WebViewport class responsible for implementing user interaction that
  a typical web view has

  Responsibilities:
   * Filters the child events of the viewport item and interprets them
   * forwards some of the events to the viewport item
   * Resizes (zooms) the viewport

  Not used in DUI atm. Maybe needed someday
*/
WebViewport::WebViewport(WebViewportItem* viewportWidget, QGraphicsItem* parent)
    : PannableViewport(parent)
    , m_recognizer(this)
    , m_selfSentEvent(0)
{
    m_recognizer.reset();

    setWidget(viewportWidget);
    connect(viewportWidget, SIGNAL(contentsSizeChangeCausedResize()), this, SLOT(contentsSizeChangeCausedResize()));
}

WebViewportItem* WebViewport::viewportWidget() const
{
    // fixme: dont use upcast
    return qobject_cast<WebViewportItem*>(pannedWidget());
}

bool WebViewport::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    // avoid filtering events that are self-sent
    if (e == m_selfSentEvent) {
        return false;
    }

    /* Apply super class event filter. This will capture mouse
    move for panning.  it will return true when applies panning
    but false until pan events are recognized
    */
    bool doFilter = PannableViewport::sceneEventFilter(i, e);

    if (!isVisible())
        return doFilter;

    switch (e->type()) {
    case QEvent::GraphicsSceneContextMenu:
        // filter out context menu, comes from long tap
        doFilter = true;
        break;

    case QEvent::GraphicsSceneHoverMove:
    case QEvent::GraphicsSceneHoverLeave:
    case QEvent::GraphicsSceneHoverEnter:
        // filter out hover, so that we don't get excess
        // link highlights while panning
        doFilter = true;
        break;

    case QEvent::GraphicsSceneMousePress:
        // mouse press is not filtered before pan is recognized
        // apply own filtering
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick:
        // FIXME: detect interaction properly
        viewportWidget()->setResizeMode(WebViewportItem::ResizeWidgetToContent);
        m_recognizer.filterMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        doFilter = true;
        break;

    case QEvent::GraphicsSceneWheel:
        // filter out wheel events that go directly to child.
        // divert the events to this object
        wheelEventFromChild(static_cast<QGraphicsSceneWheelEvent *>(e));
        doFilter = true;
        break;

    default:
        break;
    }

    return doFilter;

}

/*! \reimp reimplemented from \QAbstractKineticScroller
 */
void WebViewport::cancelLeftMouseButtonPress(const QPoint &)
{
    // don't send the mouse press event after this callback.
    // QAbstractCKineticScroller started panning
    m_recognizer.clearDelayedPress();
}

void WebViewport::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QRectF target;
    WebViewportItem* vi = viewportWidget();

    // viewport is center is the target hotspot
    QPointF viewTargetHotspot(size().width() / 2, size().height() / 2);

    if (isZoomedIn()) {
        startZoomAnimToItemHotspot(QPointF(0, -vi->pos().y()), viewTargetHotspot, size().width() / vi->size().width());
    } else {
        QPointF p = vi->mapFromScene(event->scenePos());
        target = vi->findZoomableRectForPoint(p);
        if (!target.isValid()) {
            // fixme
            return;
        }

        // hotspot is the center of the identified rect x-wise
        // y-wise it's the place user touched
        QPointF hotspot(target.center().x(), p.y());


        startZoomAnimToItemHotspot(hotspot, viewTargetHotspot, size().width() / target.size().width());
    }
}

void WebViewport::mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent * event)
{
    QGraphicsSceneMouseEvent mappedEvent(event->type());
    mapEventCommonProperties(this, event, mappedEvent);
    copyMouseEvent(event, &mappedEvent);

    event->accept();

    mouseDoubleClickEvent(&mappedEvent);
}

void WebViewport::mousePressEventFromChild(QGraphicsSceneMouseEvent * event)
{
    m_selfSentEvent = event;
    QApplication::sendEvent(scene(), event);
    m_selfSentEvent = 0;
}

void WebViewport::mouseReleaseEventFromChild(QGraphicsSceneMouseEvent * event)
{
    m_selfSentEvent = event;
    QApplication::sendEvent(scene(), event);
    m_selfSentEvent = 0;
}


void WebViewport::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    int adv = event->delta() / (15*8);
    qreal scale = 1;
    if (adv > 0) {
        scale += adv * s_zoomScaleWheelStep;
    } else {
        scale += (-adv) * s_zoomScaleWheelStep;
        scale = 1/scale;
    }

    // zoom hotspot is the point where user had the mouse
    QPointF hotspot(viewportWidget()->mapFromScene(event->scenePos()));

    // maintain that spot in the same point on the viewport
    QPointF viewTargetHotspot(viewportWidget()->mapToParent(hotspot));

    startZoomAnimToItemHotspot(hotspot, viewTargetHotspot, scale);
    event->accept();
}

void WebViewport::wheelEventFromChild(QGraphicsSceneWheelEvent *event)
{
    QGraphicsSceneWheelEvent mappedEvent(event->type());
    mapEventCommonProperties(this, event, mappedEvent);

    mappedEvent.setDelta(event->delta());
    mappedEvent.setOrientation(event->orientation());

    event->accept();

    wheelEvent(&mappedEvent);
}

bool WebViewport::mouseEventFromChild(QGraphicsSceneMouseEvent *event)
{
    return m_recognizer.filterMouseEvent(event);
}

/*!
  \targetRect in viewport item coords
*/
void WebViewport::startZoomAnimToItemHotspot(const QPointF& hotspot, const QPointF& viewTargetHotspot, qreal scale)
{
    WebViewportItem* vi = viewportWidget();

    QPointF newHotspot = (hotspot * scale);

    QPointF newViewportOrigo = newHotspot - viewTargetHotspot;

    QRectF r(- newViewportOrigo, vi->size() * scale);

    // mark that interaction has happened
    viewportWidget()->setResizeMode(WebViewportItem::ResizeWidgetToContent);

    startPannedWidgetGeomAnim(r);

}

bool WebViewport::isZoomedIn() const
{
    return size().width() < viewportWidget()->size().width();
}

void WebViewport::reset()
{
    stopPannedWidgetGeomAnim();
    // mark that interaction has not happened
    viewportWidget()->setResizeMode(WebViewportItem::ResizeWidgetHeightToContent);
    viewportWidget()->resize(viewportWidget()->contentsSize() * (size().width() / viewportWidget()->contentsSize().width()));
}

void WebViewport::contentsSizeChangeCausedResize()
{
    stopPannedWidgetGeomAnim();
}


