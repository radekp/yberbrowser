1/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QNetworkRequest>
#include <QTextStream>
#include <QVector>
#include <QtGui>
#include <QtNetwork/QNetworkProxy>
#include <cstdio>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>
#include <QGLWidget>
#include <QtGlobal>

#include "WebViewportItem.h"
#include "ProgressWidget.h"
#include "ScrollbarItem.h"
#include "EventHelpers.h"

static const int s_zoomAnimDurationMS = 300;
static const int s_zoomCommitTimerDurationMS = 500;

static const float s_zoomScaleWheelStep = .2;
static const qreal s_minZoomScale = .01; // arbitrary
static const qreal s_maxZoomScale = 10.; // arbitrary

static const int s_minDoubleClickZoomTargetWidth = 100; // in document coords, aka CSS pixels

static const int s_defaultPreferredWidth = 1024;
static const int s_defaultPreferredHeight = 768;

static const unsigned s_tileSize = 35;

#define TILE_KEY(x,y) (x << 16 | y)

class TileItem : public QObject {
    Q_OBJECT
public:
    TileItem(unsigned hPos, unsigned vPos, QGraphicsItem* parent);
    ~TileItem();

    bool isActive() const;
    void setActive(bool active);
    void painted();

private Q_SLOTS:
    void paintBlinkEnd();

private:    
    unsigned           m_vPos;
    unsigned           m_hPos;
    bool               m_active;
    unsigned           m_beingPainted;
    QGraphicsRectItem* m_rectItem;
};

TileItem::TileItem(unsigned hPos, unsigned vPos, QGraphicsItem* parent) 
    : m_vPos(vPos)
    , m_hPos(hPos)
    , m_active(false)
    , m_beingPainted(0)
    , m_rectItem(new QGraphicsRectItem(hPos*s_tileSize, vPos*s_tileSize, s_tileSize, s_tileSize, parent))
{
    setActive(true);
}

TileItem::~TileItem()
{
    delete m_rectItem;
}

bool TileItem::isActive() const 
{ 
    return m_active; 
}

void TileItem::setActive(bool active) 
{
    if (active && m_active)
        qDebug() << "duplicate tile at:" << m_hPos << " " <<  m_vPos;
    
    m_active = active;
    m_rectItem->setBrush(QBrush(active?Qt::cyan:Qt::gray));
    m_rectItem->setOpacity(0.4);
}

void TileItem::painted() 
{
    if (!m_rectItem->isVisible() || !m_active || m_beingPainted)
        return;
    m_beingPainted = 1;
    m_rectItem->setBrush(QBrush(Qt::red));
    QTimer::singleShot(1000, this, SLOT(paintBlinkEnd()));
}

void TileItem::paintBlinkEnd() 
{
    // dont keep getting reinvalidated 
    if (m_beingPainted == 1) {
        m_rectItem->setBrush(QBrush(m_active?Qt::cyan:Qt::gray));
        QTimer::singleShot(1000, this, SLOT(paintBlinkEnd()));
        m_beingPainted = 2;
        return;
    }
    m_beingPainted = 0;
}

WebViewportItem::WebViewportItem(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_webView(0)
    , m_zoomAnim(this)
    , m_zoomCommitTimer(this)
    , m_recognizer(this)
    , m_progressBox(0)
    , m_vScrollbar(0)
    , m_hScrollbar(0)
{
#if !defined(ENABLE_PAINT_DEBUG)
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFiltersChildEvents(true);

    m_zoomAnim.setTimeLine(new QTimeLine(s_zoomAnimDurationMS));
    connect(m_zoomAnim.timeLine(), SIGNAL(stateChanged(QTimeLine::State)), this, SLOT(panAnimStateChanged(QTimeLine::State)));

    connect(&m_zoomCommitTimer, SIGNAL(timeout()), this, SLOT(commitZoom()));
    m_zoomCommitTimer.setSingleShot(true);

    resetState(true);
    setDirectionErrorMargin(100);
    
}

WebViewportItem::~WebViewportItem()
{
    resetCacheTiles();
    delete m_progressBox;
    delete m_vScrollbar;
    delete m_hScrollbar;
}

void WebViewportItem::panAnimStateChanged(QTimeLine::State newState)
{
    switch(newState) {
    case QTimeLine::Running:
        m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
        break;
    case QTimeLine::NotRunning: {
        transferAnimStateToView();
        m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
        break;
    }
    case QTimeLine::Paused:
        // ### what to do?
    default:
        break;
    }
}

QPointF WebViewportItem::clipPointToViewport(const QPointF& p, qreal targetZoomScale) const
{
    QSizeF contentsSize = m_webView->page()->mainFrame()->contentsSize() * targetZoomScale;
    QSizeF sz = size();

    qreal minX = -qMax(contentsSize.width() - sz.width(), static_cast<qreal>(0.));
    qreal minY = -qMax(contentsSize.height() - sz.height(), static_cast<qreal>(0.));

    return QPointF(qBound(minX, p.x(), static_cast<qreal>(0.)),
                   qBound(minY, p.y(), static_cast<qreal>(0.)));
}


void WebViewportItem::touchGestureBegin(const QPointF&)
{
}

void WebViewportItem::touchGestureEnd()
{
}

void WebViewportItem::startInteraction()
{
}

void WebViewportItem::stopInteraction()
{
}

void WebViewportItem::tapGesture(QGraphicsSceneMouseEvent* pressEventLike, QGraphicsSceneMouseEvent* releaseEventLike)
{
    scene()->sendEvent(m_webView, pressEventLike);
    scene()->sendEvent(m_webView, releaseEventLike);
}

void WebViewportItem::doubleTapGesture(QGraphicsSceneMouseEvent* pressEventLike)
{
    qreal curScale = m_webView->scale();
    QSize contentsSize = m_webView->page()->mainFrame()->contentsSize();
    QSizeF viewportSize = size();

    qreal targetScale;
    QPointF targetPoint;

    if (isZoomedIn()) {
        targetScale = static_cast<qreal>(viewportSize.width()) / contentsSize.width();
        targetPoint = QPointF(0, (m_webView->pos().y()/curScale)*targetScale);
    } else {
        QPointF p = m_webView->mapFromScene(pressEventLike->scenePos());

        // assume to find atleast something
        targetScale = zoomScale() + s_zoomScaleWheelStep;

        QWebHitTestResult r = m_webView->page()->mainFrame()->hitTestContent(p.toPoint());
        QWebElement e = r.enclosingBlockElement();

        while (!e.isNull() && e.geometry().width() < s_minDoubleClickZoomTargetWidth) {
            e = e.parent();
        }
        if (!e.isNull()) {
            QRectF er = e.geometry();
            er.adjust(-5, -5, 5, 5);
            qreal overMinWidth = er.width() - 300;
            if (overMinWidth < 0)
                er.adjust(overMinWidth / 2, 0, -overMinWidth / 2, 0);
            QSizeF targetSize = er.size();
            p.setX(er.x() + er.size().width() / 2);
            targetScale = static_cast<qreal>(viewportSize.width()) / targetSize.width();
        }

        targetPoint = clipPointToViewport(QPointF(viewportSize.width()/2, viewportSize.height()/2) - (p * targetScale), targetScale);
    }

    startZoomAnimTo(targetPoint, targetScale);
}

bool WebViewportItem::isZoomedIn() const
{
    QSize contentsSize = m_webView->page()->mainFrame()->contentsSize();
    qreal targetScale = static_cast<qreal>(size().width()) / contentsSize.width();
    return zoomScale() > targetScale;
}

void WebViewportItem::startZoomAnimTo(const QPointF& targetPoint, qreal targetScale)
{
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
}

void WebViewportItem::transferAnimStateToView()
{
    Q_ASSERT(m_zoomAnim.timeLine()->state() == QTimeLine::NotRunning);

    qreal step = m_zoomAnim.timeLine()->currentValue();

    qreal s = m_zoomAnim.horizontalScaleAt(step);
    QPointF p = m_zoomAnim.posAt(step);

    resetZoomAnim();

    setWebViewPos(p);
    m_webView->setScale(s);
}


qreal WebViewportItem::zoomScale() const
{
    if (!m_webView)
        return 1.;

    return m_zoomAnim.horizontalScaleAt(1) * m_webView->scale();
}

void WebViewportItem::setZoomScale(qreal value, bool commitInstantly)
{
    value = qBound(s_minZoomScale, value, s_maxZoomScale);
    qreal curZoomScale = zoomScale();

    if (value != curZoomScale) {
        QPointF p = m_webView->pos();
        m_webView->setScale(value);
        p *= value / curZoomScale;
        setWebViewPos(clipPointToViewport(p, zoomScale()));
    }

    if (commitInstantly) {
        commitZoom();
    } else {
        m_webView->setTileCacheState(QWebFrame::TileCacheNoTileCreation);
        m_zoomCommitTimer.start(s_zoomCommitTimerDurationMS);
    }
}

void WebViewportItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    int adv = event->delta() / (15*8);
    qreal newScale = zoomScale() + adv * s_zoomScaleWheelStep;

    QPointF p = clipPointToViewport(m_webView->mapFromScene(event->scenePos()), newScale);

    startZoomAnimTo(p, newScale);
    event->accept();
}

void WebViewportItem::commitZoom()
{
    qreal s = m_webView->scale();
    m_webView->setTileCacheZoomFactorX(s);
    m_webView->setTileCacheZoomFactorY(s);
    m_webView->setTileCacheState(QWebFrame::TileCacheNormal);
}

void WebViewportItem::setWebView(QGraphicsWebView* view)
{
    if (m_webView) {
        m_webView->setParentItem(0);
        delete m_webView;
    }

    m_webView = view;
    m_webView->setParentItem(this);
    m_webView->setAttribute(Qt::WA_OpaquePaintEvent, true);
    m_zoomAnim.setItem(m_webView);
    delete m_progressBox;
    m_progressBox = new ProgressWidget(this);

    delete m_hScrollbar;
    m_hScrollbar = new ScrollbarItem(Qt::Horizontal, this);
    delete m_vScrollbar;
    m_vScrollbar = new ScrollbarItem(Qt::Vertical, this);

    updatePreferredSize();
}

void WebViewportItem::resetState(bool resetZoom)
{
    m_interactionState = NoInteraction;
    m_recognizer.reset();

    if (m_webView)
        setWebViewPos(QPointF(0, 0));

    m_zoomAnim.setPosAt(0, QPointF(0,0));
    m_zoomAnim.setPosAt(1, QPointF(0,0));

    if (resetZoom) {
        if (m_webView) {
            m_webView->setScale(1.);
            m_webView->setTileCacheZoomFactorX(1.);
            m_webView->setTileCacheZoomFactorY(1.);
            m_webView->setTileCacheState(QWebFrame::TileCacheNormal);
        }
        m_zoomAnim.timeLine()->stop();
        resetZoomAnim();
    }
}

void WebViewportItem::resetZoomAnim()
{
    // we have to clear the anim transforms
    m_zoomAnim.clear();
    // calling just QGraphicsItemAnimation::clear(),setPos(0) does
    // not work. it leaves the scaling there
    m_zoomAnim.setScaleAt(0, 1., 1.);
    m_zoomAnim.setPosAt(0, QPointF(0,0));
    m_zoomAnim.setStep(0);
}

#if defined(ENABLE_PAINT_DEBUG)
void WebViewportItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    static int n = 0;
    ++n;
    qDebug() << "WebViewportItem::paint" << option->exposedRect.toRect() << boundingRect() << (n % 2 ? "Qt::green" : "Qt::darkGreen");

    painter->save();
    painter->setPen(Qt::green);
    painter->setBrush(Qt::green);
    painter->fillRect(option->exposedRect.toRect(), n % 2 ? Qt::green : Qt::darkGreen );
    painter->restore();
    QGraphicsWidget::paint(painter, option, widget);
}
#endif

/*!
  Copies common properties of mouse/wheel event to new event. Maps the coordinates from the event
  to the coordinate space of the receiving item

  This is very errorprone and should be replaced.
  \internal
 */
template<typename EventType>
void mapEventCommonProperties(QGraphicsItem* parent, const EventType* event, EventType& mappedEvent)
{
    // lifted form qmlgraphicsflickable.cpp, LGPL Nokia

    QRectF parentRect = parent->mapToScene(parent->boundingRect()).boundingRect();

    mappedEvent.setAccepted(false);
    mappedEvent.setPos(parent->mapFromScene(event->scenePos()));
    mappedEvent.setScenePos(event->scenePos());
    mappedEvent.setScreenPos(event->screenPos());
    mappedEvent.setModifiers(event->modifiers());
    mappedEvent.setButtons(event->buttons());
}

bool WebViewportItem::sendMouseEventFromChild(QGraphicsSceneMouseEvent *event)
{
    QGraphicsSceneMouseEvent mappedEvent(event->type());
    mapEventCommonProperties(this, event, mappedEvent);

    for (int i = 0x1; i <= 0x10; i <<= 1) {
        if (event->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            mappedEvent.setButtonDownPos(button, mapFromScene(event->buttonDownPos(button)));
            mappedEvent.setButtonDownScenePos(button, event->buttonDownScenePos(button));
            mappedEvent.setButtonDownScreenPos(button, event->buttonDownScreenPos(button));
        }
    }

    mappedEvent.setLastPos(mapFromScene(event->lastScenePos()));
    mappedEvent.setLastScenePos(event->lastScenePos());
    mappedEvent.setLastScreenPos(event->lastScreenPos());
    mappedEvent.setButton(event->button());

    return m_recognizer.filterMouseEvent(event);
}

bool WebViewportItem::sendWheelEventFromChild(QGraphicsSceneWheelEvent *event)
{
    QGraphicsSceneWheelEvent mappedEvent(event->type());
    mapEventCommonProperties(this, event, mappedEvent);

    mappedEvent.setDelta(event->delta());
    mappedEvent.setOrientation(event->orientation());

    wheelEvent(&mappedEvent);
    return mappedEvent.isAccepted();
}

bool WebViewportItem::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    // lifted form qmlgraphicsflickable.cpp, LGPL Nokia
    if (!isVisible())
        return QGraphicsItem::sceneEventFilter(i, e);

    bool doFilter = false;

    switch (e->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        doFilter = handleMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        if (!doFilter)
            doFilter = sendMouseEventFromChild(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;

    case QEvent::GraphicsSceneWheel:
        doFilter = sendWheelEventFromChild(static_cast<QGraphicsSceneWheelEvent*>(e));
        break;

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

    default:
        break;
    }

    // prevent text selection and image draggin
    if (e->type() == QEvent::GraphicsSceneMouseMove)
        return true;

    return doFilter ? true : QGraphicsItem::sceneEventFilter(i, e);
}

QGraphicsWebView* WebViewportItem::webView()
{
    return m_webView;
}

void WebViewportItem::setGeometry(const QRectF& rect)
{
    QGraphicsWidget::setGeometry(rect);
    if (m_progressBox)
        m_progressBox->udpateGeometry(rect);
    updateScrollbars();
}

void WebViewportItem::updatePreferredSize()
{
    // FIXME: we have bug in QtWebKit API when tileCacheEnabled is true.
    // this causes viewport not to reset between the page loads.
    // Thus, we need to update viewport manually until we have fix for this.

    m_webView->page()->setPreferredContentsSize(QSize(s_defaultPreferredWidth, s_defaultPreferredHeight));
}

void WebViewportItem::setPanPos(const QPointF& pos)
{
    setWebViewPos(clipPointToViewport(pos, zoomScale()));
}

QPointF WebViewportItem::panPos() const
{
    return m_webView->pos();
}

void WebViewportItem::showTiles(bool tilesOn) 
{
    if (tilesOn) {
        connect(m_webView->page(),SIGNAL(tileCreated(unsigned, unsigned)), this, SLOT(tileCreated(unsigned, unsigned)));
        connect(m_webView->page(),SIGNAL(tileRemoved(unsigned, unsigned)), this, SLOT(tileRemoved(unsigned, unsigned)));
        connect(m_webView->page(),SIGNAL(tilePainted(unsigned, unsigned)), this, SLOT(tilePainted(unsigned, unsigned)));
        connect(m_webView->page(),SIGNAL(tileCacheViewportScaleChanged()), this, SLOT(tileCacheViewportScaleChanged()));
    } else {
        disconnect(m_webView->page(),SIGNAL(tileCreated(unsigned, unsigned)), this, SLOT(tileCreated(unsigned, unsigned)));
        disconnect(m_webView->page(),SIGNAL(tileRemoved(unsigned, unsigned)), this, SLOT(tileRemoved(unsigned, unsigned)));
        disconnect(m_webView->page(),SIGNAL(tilePainted(unsigned, unsigned)), this, SLOT(tilePainted(unsigned, unsigned)));
        disconnect(m_webView->page(),SIGNAL(tileCacheViewportScaleChanged()), this, SLOT(tileCacheViewportScaleChanged()));
        // empty the tile map
        resetCacheTiles();
    }
}

void WebViewportItem::tileCreated(unsigned hPos, unsigned vPos)
{
    // new tile or just inactive?
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos)))
        m_tileMap.insert(TILE_KEY(hPos, vPos), new TileItem(hPos, vPos, this));
    else
        m_tileMap.value(TILE_KEY(hPos, vPos))->setActive(true);
}

void WebViewportItem::tileRemoved(unsigned hPos, unsigned vPos)
{
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos)))
        return;
    m_tileMap.value(TILE_KEY(hPos, vPos))->setActive(false);
}

void WebViewportItem::tilePainted(unsigned hPos, unsigned vPos)
{
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos)))
        tileCreated(hPos, vPos);
    m_tileMap.value(TILE_KEY(hPos, vPos))->painted();
}

void WebViewportItem::tileCacheViewportScaleChanged()
{
    QTimer::singleShot(0, this, SLOT(resetCacheTiles()));
}

void WebViewportItem::resetCacheTiles()
{
    QMapIterator<int, TileItem*> i(m_tileMap);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }    
    m_tileMap.clear();
}

void WebViewportItem::setWebViewPos(const QPointF& point)
{
    m_webView->setPos(point);
    updateScrollbars();
}

void WebViewportItem::updateScrollbars()
{
    if (!m_vScrollbar || !m_hScrollbar)
        return;
    QPointF contentPos = m_webView->pos();
    QSizeF contentSize = m_webView->page()->mainFrame()->contentsSize() * zoomScale();

    QSizeF viewSize = size();

    bool shouldFadeOut = !(state() == QAbstractKineticScroller::MousePressed || state() == QAbstractKineticScroller::Pushing);

    m_hScrollbar->contentPositionUpdated(contentPos.x(), contentSize.width(), viewSize, shouldFadeOut);
    m_vScrollbar->contentPositionUpdated(contentPos.y(), contentSize.height(), viewSize, shouldFadeOut);
}

QPoint WebViewportItem::maximumScrollPosition() const
{
    QSizeF contentsSize = m_webView->page()->mainFrame()->contentsSize() * zoomScale();
    QSizeF sz = size();
    QSize maxSize = (contentsSize - sz).toSize();

    return QPoint(qMax(0, maxSize.width()), qMax(0, maxSize.height()));
}

QSize WebViewportItem::viewportSize() const
{
    return size().toSize();
}

QPoint WebViewportItem::scrollPosition() const
{
    qDebug() << __FUNCTION__ << panPos() << m_overShootDelta  << " max" << maximumScrollPosition();

    return (-(panPos()-m_overShootDelta)).toPoint();
}

void WebViewportItem::setScrollPosition(const QPoint &pos, const QPoint &overShootDelta)
{
    qDebug() << __FUNCTION__ << pos << overShootDelta;
    m_overShootDelta = overShootDelta;
    setWebViewPos(-(pos - overShootDelta));
}

void WebViewportItem::stateChanged(QAbstractKineticScroller::State oldState)
{
    QAbstractKineticScroller::stateChanged(oldState);
    updateScrollbars();
}

bool WebViewportItem::canStartScrollingAt(const QPoint &globalPos) const
{
    return QAbstractKineticScroller::canStartScrollingAt(globalPos);
}

void WebViewportItem::cancelLeftMouseButtonPress(const QPoint &globalPressPos)
{
    QAbstractKineticScroller::cancelLeftMouseButtonPress(globalPressPos);

}


#include "WebViewportItem.moc"
