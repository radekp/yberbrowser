#include <QApplication>
#include <QGraphicsScene>
#include "WebViewport.h"
#include "WebViewportItem.h"
#include "EventHelpers.h"
#include <QDebug>

namespace {
const int s_zoomAnimDurationMS = 300;

const float s_zoomScaleWheelStep = .2;
const int s_doubleClickWaitTimeout = 100;
}



WebViewport::WebViewport(QGraphicsItem* parent)
    : PannableViewport(parent)
    , m_recognizer(this)
    , m_selfSentEvent(0)
    , m_zoomAnim(this)
{
    m_recognizer.reset();

    m_zoomAnim.setTimeLine(new QTimeLine(s_zoomAnimDurationMS));
    connect(m_zoomAnim.timeLine(), SIGNAL(stateChanged(QTimeLine::State)), this, SLOT(zoomAnimStateChanged(QTimeLine::State)));

}

WebViewportItem* WebViewport::viewportItem() const
{
    // fixme: dont use upcast
    return qobject_cast<WebViewportItem*>(pannedWidget());
}

bool WebViewport::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    qDebug() << __PRETTY_FUNCTION__ << e;

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
    WebViewportItem* vi = viewportItem();

    if (isZoomedIn()) {
        startZoomAnimToItemHotspot(QPointF(0, -vi->pos().y()), size().width() / vi->size().width());
    } else {
        QPointF p = vi->mapFromScene(event->scenePos());
        qDebug() << "POS" << p;
        target = vi->findZoomableRectForPoint(p);
        if (!target.isValid()) {
            // fixme
            return;
        }
        startZoomAnimToItemHotspot(p, vi->size().width() / target.size().width());
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
    qDebug() << __PRETTY_FUNCTION__ << event;

    int adv = event->delta() / (15*8);
    qreal scale = 1;
    if (adv > 0) {
        scale += adv * s_zoomScaleWheelStep;
    } else {
        scale += (-adv) * s_zoomScaleWheelStep;
        scale = 1/scale;
    }

    startZoomAnimToItemHotspot(viewportItem()->mapFromScene(event->scenePos()), scale);
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

void WebViewport::zoomAnimStateChanged(QTimeLine::State newState)
{
    switch(newState) {
    case QTimeLine::Running:
        viewportItem()->disableContentUpdates();
        break;

    case QTimeLine::NotRunning: {
        transferAnimStateToView();
        break;
    }
    case QTimeLine::Paused:
        // FIXME: what to do?
        break;
    default:
        break;
    }
}
/*!
  \targetRect in viewport item coords
*/
void WebViewport::startZoomAnimToItemHotspot(const QPointF& hotspot, qreal scale)
{
    WebViewportItem* vi = viewportItem();

    QPointF p(vi->mapToParent(hotspot));

    QPointF newHotspot = (hotspot * scale);

    QPointF newViewportOrigo = newHotspot - p;

    QRectF r(- newViewportOrigo, vi->size() * scale);

    qDebug() << "target:" << hotspot << "p:"  << p << "newHot:" <<newHotspot << " current origo:" << vi->mapToParent(QPoint()) <<" newOrigo:"<<newViewportOrigo << " calc:" << r << " scaling: " << scale;

    viewportItem()->setGeometry(r);

#if 0
    m_zoomAnim.timeLine()->stop();
    qreal step = m_zoomAnim.timeLine()->currentValue();

    qreal curScale = m_zoomAnim.horizontalScaleAt(step) * m_webView->scale();
    QPointF curPos = m_zoomAnim.posAt(step) + m_webView->pos();

    setWebViewPos(QPointF(0,0));
    m_webView->setScale(1.);

    m_zoomAnim.setPosAt(0, curPos);
    m_zoomAnim.setPosAt(1, targetPoint);
    m_zoomAnim.setScaleAt(0, curScale, curScale);
    m_zoomAnim.setScaleAt(1, targetScale, targetScale);
    m_zoomAnim.setStep(0);
    m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
    m_zoomAnim.timeLine()->start();
#endif
}

void WebViewport::transferAnimStateToView()
{
#if 0
    Q_ASSERT(m_zoomAnim.timeLine()->state() == QTimeLine::NotRunning);

    qreal step = m_zoomAnim.timeLine()->currentValue();

    qreal s = m_zoomAnim.horizontalScaleAt(step);
    QPointF p = m_zoomAnim.posAt(step);

    resetZoomAnim();

    setWebViewPos(p);
    m_webView->setScale(s);
#endif
}

bool WebViewport::isZoomedIn() const
{
    return size().width() < viewportItem()->size().width();
}

