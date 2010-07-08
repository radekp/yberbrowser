/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <QApplication>
#include "EventHelpers.h"
#include "LinkSelectionItem.h"
#include "WebView.h"
#include "WebViewport.h"
#include "WebViewportItem.h"
#include "qwebframe.h"
#include "qgraphicswebview.h"
#include "qwebelement.h"

#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QMetaMethod>

 #include <QGraphicsSceneResizeEvent>

//#define ENABLE_LINK_SELECTION_DEBUG

namespace {
const float s_zoomScaleWheelStep = .2;
const int s_edgeMarginForLinkSelectionRetry = 50;
const int s_minSearchRectSize = 8;
const int s_maxSearchRectSize = 25;
const int backingStoreUpdateEnableDelay = 700;
const int s_geomAnimDuration = 300;

}

/*!
  \class WebViewport class responsible for implementing user interaction that
  a typical web view has

  Responsibilities:
   * Filters the child events of the viewport item and interprets them
   * forwards some of the events to the viewport item
   * Resizes (zooms) the viewport

  Not used in Meego atm. Maybe needed someday
*/
WebViewport::WebViewport(WebViewportItem* viewportWidget, QGraphicsItem* parent)
    : PannableViewport(parent)
    , m_viewportWidget(viewportWidget)
    , m_panningState(WebViewport::Inactive)
    , m_recognizer(this)
    , m_selfSentEvent(0)
    , m_linkSelectionItem(0)
    , m_delayedMouseReleaseEvent(0)
#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
    , m_searchRectItem(0)
    , m_clickablePointItem(0)
#endif
{
    // AutoRange is set to false, because MPannableViewport observes
    // sizehints, not the size of the contained object
    // setting sizehints for every resize doesn't seem to work
    // for WebViewportItem
    setAutoRange(false);
    setPanDirection(Qt::Horizontal | Qt::Vertical);
    setWidget(m_viewportWidget);

    m_geomAnim.addAnimation(&m_posAnim);
    m_geomAnim.addAnimation(&m_sizeAnim);

    m_posAnim.setTargetObject(this);
    m_posAnim.setDuration(s_geomAnimDuration);
    m_posAnim.setPropertyName("position");
    m_sizeAnim.setTargetObject(m_viewportWidget);
    m_sizeAnim.setDuration(s_geomAnimDuration);
    m_sizeAnim.setPropertyName("size");

    connect(&m_geomAnim, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)), this, SLOT(geomAnimStateChanged(QAbstractAnimation::State,QAbstractAnimation::State)));

    connect(viewportWidget, SIGNAL(contentsSizeChangeCausedResize()), this, SLOT(contentsSizeChangeCausedResize()));
    connect(viewportWidget, SIGNAL(zoomRectForPointReceived(const QPointF&, const QRectF&)), SLOT(zoomRectForPointReceived(const QPointF&, const QRectF&)));
    m_backingStoreUpdateEnableTimer.setSingleShot(true);
    connect(&m_backingStoreUpdateEnableTimer, SIGNAL(timeout()), this, SLOT(enableBackingStoreUpdates()));
    connect(this, SIGNAL(positionChanged(QPointF)), this, SLOT(webPanningStarted()));
    connect(this, SIGNAL(panningStopped()), this, SLOT(webPanningStopped()));
}

WebViewport::~WebViewport()
{
    delete m_delayedMouseReleaseEvent;
    delete m_linkSelectionItem;

#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
    delete m_searchRectItem;
    delete m_clickablePointItem;
#endif
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
        m_viewportWidget->setResizeMode(WebViewportItem::ContentResizePreservesScale);
        m_recognizer.filterMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        doFilter = true;
        break;

    case QEvent::GraphicsSceneWheel:
        // filter out wheel events that go directly to child.
        // divert the events to this object
        wheelEventFromChild(static_cast<QGraphicsSceneWheelEvent *>(e));
        doFilter = true;
        break;

#if defined(Q_WS_MAEMO_5)
    case QEvent::KeyPress:
        doFilter = processMaemo5ZoomKeys(static_cast<QKeyEvent*>(e));
        break;
#endif

    default:
        break;
    }

    return doFilter;

}

void WebViewport::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    // viewport is center is the target hotspot
    QPointF viewTargetHotspot(size().width() / 2, size().height() / 2);

    if (isZoomedIn())
        startZoomAnimToItemHotspot(QPointF(0, -m_viewportWidget->pos().y()), viewTargetHotspot, size().width() / m_viewportWidget->size().width());
    else
        m_viewportWidget->findZoomableRectForPoint(m_viewportWidget->mapFromScene(event->scenePos()));
}

void WebViewport::zoomRectForPointReceived(const QPointF& originalPos, const QRectF& target)
{
    QPointF viewTargetHotspot(size().width() / 2, size().height() / 2);
    if (!target.isValid()) {
        // fixme
        return;
    }
    // hotspot is the center of the identified rect x-wise
    // y-wise it's the place user touched
    QPointF hotspot(target.center().x(), originalPos.y());
    startZoomAnimToItemHotspot(hotspot, viewTargetHotspot, size().width() / target.size().width());
}

void WebViewport::mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent* event)
{
    QGraphicsSceneMouseEvent mappedEvent(event->type());
    mapEventCommonProperties(this, event, mappedEvent);
    copyMouseEvent(event, &mappedEvent);

    event->accept();

    mouseDoubleClickEvent(&mappedEvent);
}

void WebViewport::mousePressEventFromChild(QGraphicsSceneMouseEvent* event)
{
    // FIXME: setpos for release event should be adjusted somewhere else.
    event->setPos(m_viewportWidget->webView()->mapFromScene(event->scenePos()));
#if defined(ENABLE_LINK_SELECTION_DEBUG)
    qDebug() << __FUNCTION__ << " mouse press pos:" << event->pos() << " scene pos: " << event->scenePos();
#endif
    m_selfSentEvent = event;
    QApplication::sendEvent(scene(), event);
    m_selfSentEvent = 0;
}

QWebFrame* findFrame(const QPoint& pos, QWebFrame* frame)
{
    if (!frame)
        return 0;
    QList<QWebFrame*> children = frame->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        if (findFrame(pos, children.at(i)))
            return children.at(i);
    }

    if (frame->geometry().contains(pos))
        return frame;
    return 0;
}

void WebViewport::adjustClickPosition(QPointF& pos)
{
#if USE_WEBKIT2
    Q_UNUSED(pos);
#else
    QPointF localPos = m_viewportWidget->webView()->mapFromScene(pos);
#if defined(ENABLE_LINK_SELECTION_DEBUG)
    qDebug() << __FUNCTION__ << " click pos:" << localPos << " scene pos:" << pos;
#endif
	QPoint pp(localPos.x(), localPos.y());
    QWebFrame* qframe = findFrame(pp, m_viewportWidget->webView()->page()->mainFrame());
    if (!qframe)
        return;

    static int methodOffset = qframe->metaObject()->indexOfMethod("findClickableNode(QRect,QPoint&)");
    if (methodOffset < 0)
        return;
    static QMetaMethod findClickableNodeMethod = qframe->metaObject()->method(methodOffset);

    QPoint resultPoint;
    // zoom dependent search rect size
    int searchDist = qMin(qMax(s_minSearchRectSize, (int)(10 / m_viewportWidget->zoomScale())), s_maxSearchRectSize);
    // non-square shape search rect
    QRect searchRect(pp.x() - searchDist, pp.y() - 2*searchDist, 2*searchDist, 4*searchDist);

#if defined(ENABLE_LINK_SELECTION_DEBUG)
    qDebug() << "clicked frame found:"<< qframe << " zoomscale:" << m_viewportWidget->zoomScale() << " search rect:" << searchRect;
#endif

#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
    delete m_searchRectItem;
    m_searchRectItem = new QGraphicsRectItem(QRectF(m_viewportWidget->webView()->mapToScene(searchRect).boundingRect()), this);
    delete m_clickablePointItem; m_clickablePointItem = 0;
#endif

    bool found;
    findClickableNodeMethod.invoke(qframe, Q_RETURN_ARG(bool, found), Q_ARG(QRect, searchRect), Q_RETURN_ARG(QPoint, resultPoint));
    if (found) {
        pos = m_viewportWidget->webView()->mapToScene(resultPoint);
#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
        m_clickablePointItem = new QGraphicsEllipseItem(m_viewportWidget->webView()->mapToScene(QRect(resultPoint.x() - 3, resultPoint.y() - 3, 6, 6)).boundingRect(), this);
#endif
#if defined(ENABLE_LINK_SELECTION_DEBUG)
        qDebug() << "clickable node found at:" << resultPoint;
#endif
    } else {
#if defined(ENABLE_LINK_SELECTION_DEBUG)
        qDebug() << "clickable node NOT found, see if we need to go closer to the edge";
#endif
        // vertical adjust
        if (pos.y() < s_edgeMarginForLinkSelectionRetry)
            searchRect.moveTop(searchRect.top() - pos.y());
        else if (pos.y() > rect().bottom() - s_edgeMarginForLinkSelectionRetry)
            searchRect.moveBottom(searchRect.bottom() + (rect().bottom() - pos.y()));
        else
            return;
#if defined(ENABLE_LINK_SELECTION_DEBUG)
        qDebug() << "search again:" << searchRect;
#endif
        // search again
        qframe = findFrame(searchRect.center(), m_viewportWidget->webView()->page()->mainFrame());
        if (!qframe)
            return;
        findClickableNodeMethod.invoke(qframe, Q_RETURN_ARG(bool, found), Q_ARG(QRect, searchRect), Q_RETURN_ARG(QPoint, resultPoint));
        if (found) {
            pos = m_viewportWidget->webView()->mapToScene(resultPoint);
#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
        m_clickablePointItem = new QGraphicsEllipseItem(m_viewportWidget->webView()->mapToScene(QRect(resultPoint.x() - 3, resultPoint.y() - 3, 6, 6)).boundingRect(), this);
#endif
#if defined(ENABLE_LINK_SELECTION_DEBUG)
            qDebug() << "clickable node found at :" << resultPoint;
#endif
        }
    }
#endif
}

void WebViewport::mouseReleaseEventFromChild(QGraphicsSceneMouseEvent * event)
{
    delete m_linkSelectionItem;
    m_linkSelectionItem = 0;
    delete m_delayedMouseReleaseEvent;
    m_delayedMouseReleaseEvent = 0;

    // FIXME: setPos for release event should be adjusted somewhere else.
    event->setPos(m_viewportWidget->webView()->mapFromScene(event->scenePos()));
    QPointF p = event->pos();

#if !USE_WEBKIT2
    QWebHitTestResult result = m_viewportWidget->webView()->page()->mainFrame()->hitTestContent(QPoint(p.x(), p.y()));
    if (!result.linkElement().isNull()) {
#if defined(ENABLE_LINK_SELECTION_DEBUG)
        qDebug() << "hittest found" << p;
#endif
        QPoint linkPoint = result.boundingRect().topLeft();
        QWebFrame* frame = result.frame();
        while (frame) {
            linkPoint+=frame->pos();
            frame = frame->parentFrame();
        }
        m_linkSelectionItem = new LinkSelectionItem(this);
        m_linkSelectionItem->appear(m_viewportWidget->webView()->mapToScene(p), m_viewportWidget->webView()->mapToScene(QRect(linkPoint, result.boundingRect().size())).boundingRect());
        // delayed click
        m_delayedMouseReleaseEvent = new QGraphicsSceneMouseEvent(event->type());
        copyMouseEvent(event, m_delayedMouseReleaseEvent);
        QTimer::singleShot(500, this, SLOT(startLinkSelection()));
        return;
    } else {
#if defined(ENABLE_LINK_SELECTION_DEBUG)
        qDebug() << "hittest NOT found" << p;
#endif
    }
#endif
    m_selfSentEvent = event;
    QApplication::sendEvent(scene(), event);
    m_selfSentEvent = 0;
}

void WebViewport::startLinkSelection()
{
    if (!m_delayedMouseReleaseEvent)
        return;

    m_selfSentEvent = m_delayedMouseReleaseEvent;
    QApplication::sendEvent(scene(), m_delayedMouseReleaseEvent);
    m_selfSentEvent = 0;
    delete m_delayedMouseReleaseEvent;
    m_delayedMouseReleaseEvent = 0;
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
    QPointF hotspot(m_viewportWidget->mapFromScene(event->scenePos()));

    // maintain that spot in the same point on the viewport
    QPointF viewTargetHotspot(m_viewportWidget->mapToParent(hotspot));

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

bool WebViewport::processMaemo5ZoomKeys(QKeyEvent* event)
{
    const bool zoomIn = event->key() == Qt::Key_F7;
    const bool zoomOut = event->key() == Qt::Key_F8;

    if (!zoomIn && !zoomOut)
        return false;

    event->accept();

    qreal scale = 1;
    if (zoomIn)
        scale += s_zoomScaleWheelStep;
    else
        scale -= s_zoomScaleWheelStep;

    // zoom to the center
    QPointF center = sceneBoundingRect().center();
    QPointF viewTargetCenter(m_viewportWidget->mapToParent(center));

    startZoomAnimToItemHotspot(center, viewTargetCenter, scale);
    event->accept();

    return true;
}

/*!
  \targetRect in viewport item coords
*/
void WebViewport::startZoomAnimToItemHotspot(const QPointF& hotspot, const QPointF& viewTargetHotspot, qreal scale)
{
    QPointF newHotspot = (hotspot * scale);
    QPointF newViewportOrigo = newHotspot - viewTargetHotspot;

    // mark that interaction has happened
    m_viewportWidget->setResizeMode(WebViewportItem::ContentResizePreservesScale);
    startPannedWidgetGeomAnim(- newViewportOrigo, m_viewportWidget->size() * scale);
}

bool WebViewport::isZoomedIn() const
{
    return size().width() < m_viewportWidget->size().width();
}

void WebViewport::reset()
{
    stopPannedWidgetGeomAnim();

    // mark that interaction has not happened
    m_viewportWidget->setResizeMode(WebViewportItem::ContentResizePreservesWidth);
    updateViewportItemSizeIfDimensionPreserved();

    m_viewportWidget->commitZoom();
}

void WebViewport::contentsSizeChangeCausedResize()
{
    stopPannedWidgetGeomAnim();
    updateViewportRange();
}

void WebViewport::webPanningStarted()
{
    // turn on and off tile creating while autoscrolling
    if (m_panningState != WebViewport::Pushing) {
        m_panningState = Pushing;
        m_backingStoreUpdateEnableTimer.stop();
        m_viewportWidget->disableContentUpdates();
    }
}

void WebViewport::webPanningStopped()
{
    if (m_panningState != WebViewport::Inactive) {
        // m_viewportWidget->enableContentUpdates();
        m_panningState = Inactive;
        m_backingStoreUpdateEnableTimer.start(backingStoreUpdateEnableDelay);

    }
}

void WebViewport::enableBackingStoreUpdates()
{
    m_panningState = Inactive;
    m_viewportWidget->enableContentUpdates();
}



QRectF WebViewport::adjustRectForPannedWidgetGeometry(const QRectF& g)
{
    stopPannedWidgetGeomAnim();

    QRectF gg(g);

    QSizeF sz = g.size();
    QSizeF vsz = size();

    qreal w = vsz.width() - sz.width();
    qreal h = vsz.height() - sz.height();

    if ( w > 0 ) {
// why are we moving webcontent to the middle?
#if 0
        m_extraPos.setX(w/2);
#endif
        gg.moveLeft(0);
    } else {
#if 0
        m_extraPos.setX(0);
#endif
        if (gg.x() < w)
            gg.moveLeft(w);
        if (gg.x() > 0)
            gg.moveLeft(0);
    }

    if ( h > 0 ) {
// why are we moving webcontent to the middle?
#if 0
        m_extraPos.setY(h/2);
#endif
        gg.moveTop(0);
    } else {
#if 0
        m_extraPos.setY(0);
#endif
        if (gg.y() < h)
            gg.moveTop(h);
        if (gg.y() > 0)
            gg.moveTop(0);
    }
#if 0
    gg.translate(m_extraPos);
#endif
#if 0
    // this is disabled because of merging the function will probably
    // go anyway soon, so this edge-case need
    // not to be taken into account now
    gg.translate(m_overShootDelta);
#endif
    return gg;
}

void WebViewport::setPannedWidgetGeometry(const QRectF& g)
{
    QRectF r(adjustRectForPannedWidgetGeometry(g));

    widget()->resize(r.size());
    setPosition(r.topLeft());

    // FIXME: Consider adding a background item instead.
    QRegion viewport(geometry().toRect());
    viewport.subtracted(QRegion(widget()->geometry().toRect()));
    update(viewport.boundingRect());

    if (m_linkSelectionItem) {
        QRectF current = m_viewportWidget->geometry();
        QPointF delta = m_viewportWidget->geometry().topLeft() - current.topLeft();
        m_linkSelectionItem->moveBy(delta.x(), delta.y());
    }
    updateViewportRange();
}

void WebViewport::startPannedWidgetGeomAnim(const QPointF& pos, const QSizeF& size)
{
    QRectF currentGeometry = widget()->geometry();

    m_posAnim.setStartValue(currentGeometry.topLeft());
    m_sizeAnim.setStartValue(currentGeometry.size());

    m_geomAnimEndValue = adjustRectForPannedWidgetGeometry(QRectF(pos, size));

    m_posAnim.setEndValue(m_geomAnimEndValue.topLeft());
    m_sizeAnim.setEndValue(m_geomAnimEndValue.size());

    m_geomAnim.start();
}

void WebViewport::stopPannedWidgetGeomAnim()
{
    m_geomAnimEndValue = QRectF();
    m_geomAnim.stop();
}

void WebViewport::transferAnimStateToView()
{
    if (m_geomAnimEndValue.isValid()) {
        widget()->resize(m_geomAnimEndValue.size());
        setPosition(m_geomAnimEndValue.topLeft());
    }
}

void WebViewport::geomAnimStateChanged(QAbstractAnimation::State newState,QAbstractAnimation::State)
{
    switch(newState) {
    case QAbstractAnimation::Running:
        break;

    case QAbstractAnimation::Stopped: {
        transferAnimStateToView();
        break;
    }
    case QAbstractAnimation::Paused:
        // FIXME: what to do?
        break;
    default:
        break;
    }
}

void WebViewport::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    PannableViewport::resizeEvent(event);
    updateViewportItemSizeIfDimensionPreserved();
}

void WebViewport::updateViewportItemSizeIfDimensionPreserved()
{
    switch(m_viewportWidget->resizeMode()) {
    case WebViewportItem::ContentResizePreservesWidth:
        setPannedWidgetGeometry(QRectF(QPointF(), m_viewportWidget->contentsSize() * (size().width() / m_viewportWidget->contentsSize().width())));
        break;
    case WebViewportItem::ContentResizePreservesHeight:
        setPannedWidgetGeometry(QRectF(QPointF(), m_viewportWidget->contentsSize() * (size().height() / m_viewportWidget->contentsSize().height())));
        break;
    case WebViewportItem::ContentResizePreservesScale:
        break;
    }

}
void WebViewport::updateViewportRange()
{
    // this is called whenever viewport item is resized
    // it is called after we change the size
    // and after item itself resizes itself
    // this must be done this way, because we cannot observe
    // the resize events of the item 
    setRange(QRectF(QPoint(), widget()->geometry().size()));
}

