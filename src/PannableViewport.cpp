#include <QPointF>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include "PannableViewport.h"
#include "ScrollbarItem.h"
#include "EventHelpers.h"

namespace {
const unsigned s_scrollsPerSecond = 60;
const qreal s_axisLockThreshold = .7;
const int s_geomAnimDuration = 300;
}

/*!
  \class PannableViewport \QGraphicsItem that acts as a viewport for its children.

  Responsibilities:
  * Implements panning interaction

  Corresponds to DuiPannableViewport in DUI, but is implemented via
  \QGraphicsItem::setFiltersChildEvents

  Not used for DUI
*/

PannableViewport::PannableViewport(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_pannedWidget(0)
    , m_vScrollbar(new ScrollbarItem(Qt::Vertical, this))
    , m_hScrollbar(new ScrollbarItem(Qt::Horizontal, this))
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFiltersChildEvents(true);

    setScrollsPerSecond(s_scrollsPerSecond);
    setOvershootPolicy(YberHack_Qt::QAbstractKineticScroller::OvershootAlwaysOn);
    setAxisLockThreshold(s_axisLockThreshold);
}

PannableViewport::~PannableViewport()
{
}

void PannableViewport::setPanPos(const QPointF& pos)
{
    setPannedWidgetGeometry(QRectF(pos, pannedWidget()->size()));
}

QPointF PannableViewport::panPos() const
{
    return pannedWidget()->pos() - m_extraPos - m_overShootDelta;
}

void PannableViewport::setRange(const QRectF& )
{

}

void PannableViewport::setWidget(QGraphicsWidget* view)
{
    if (view == m_pannedWidget)
        return;

    if (m_pannedWidget) {
        m_pannedWidget->setParentItem(0);
        delete m_pannedWidget;
    }

    m_pannedWidget = view;
    m_pannedWidget->setParentItem(this);
    m_pannedWidget->stackBefore(m_vScrollbar);

    m_geomAnim.setTargetObject(m_pannedWidget);
    m_geomAnim.setDuration(s_geomAnimDuration);
    m_geomAnim.setPropertyName("geometry");
    connect(&m_geomAnim, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)), this, SLOT(geomAnimStateChanged(QAbstractAnimation::State,QAbstractAnimation::State)));

}

QPoint PannableViewport::maximumScrollPosition() const
{
    QSizeF contentsSize = m_pannedWidget->size();
    QSizeF sz = size();
    QSize maxSize = (contentsSize - sz).toSize();

    return QPoint(qMax(0, maxSize.width()), qMax(0, maxSize.height()));
}

QSize PannableViewport::viewportSize() const
{
    return size().toSize();
}

QPoint PannableViewport::scrollPosition() const
{
    return (-panPos()).toPoint();
}

void PannableViewport::updateScrollbars()
{
    if (!m_vScrollbar || !m_hScrollbar)
        return;
    QPointF contentPos = panPos();
    QSizeF contentSize = m_pannedWidget->size();

    QSizeF viewSize = size();

    bool shouldFadeOut = !(state() == YberHack_Qt::QAbstractKineticScroller::MousePressed || state() == YberHack_Qt::QAbstractKineticScroller::Pushing);

    m_hScrollbar->contentPositionUpdated(contentPos.x(), contentSize.width(), viewSize, shouldFadeOut);
    m_vScrollbar->contentPositionUpdated(contentPos.y(), contentSize.height(), viewSize, shouldFadeOut);
}

void PannableViewport::setScrollPosition(const QPoint &pos, const QPoint &overShootDelta)
{
    m_overShootDelta = overShootDelta;
    setPanPos(-pos);
}

void PannableViewport::stateChanged(YberHack_Qt::QAbstractKineticScroller::State oldState, YberHack_Qt::QAbstractKineticScroller::State newState)
{
    YberHack_Qt::QAbstractKineticScroller::stateChanged(oldState, newState);
    updateScrollbars();
}

bool PannableViewport::canStartScrollingAt(const QPoint &globalPos) const
{
    return YberHack_Qt::QAbstractKineticScroller::canStartScrollingAt(globalPos);
}

bool PannableViewport::sceneEvent(QEvent* e)
{
    bool doFilter = false;

    switch (e->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        doFilter = handleMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;
    default:
        break;
    }
    return doFilter;
}

bool PannableViewport::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    if (!isVisible())
        return QGraphicsItem::sceneEventFilter(i, e);

    bool doFilter = false;

    switch (e->type()) {
    case QEvent::GraphicsSceneMouseDoubleClick:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        doFilter = handleMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;

    default:
        break;
    }

    return doFilter ? true : QGraphicsItem::sceneEventFilter(i, e);
}

QPointF PannableViewport::clipPointToViewport(const QPointF& p) const
{
    QSizeF contentsSize = size();
    QSizeF sz = size();

    qreal minX = -qMax(contentsSize.width() - sz.width(), static_cast<qreal>(0.));
    qreal minY = -qMax(contentsSize.height() - sz.height(), static_cast<qreal>(0.));

    return QPointF(qBound(minX, p.x(), static_cast<qreal>(0.)),
                   qBound(minY, p.y(), static_cast<qreal>(0.)));
}

QRectF PannableViewport::adjustRectForPannedWidgetGeometry(const QRectF& g)
{
    stopPannedWidgetGeomAnim();

    QRectF gg(g);

    QSizeF sz = g.size();
    QSizeF vsz = size();

    qreal w = vsz.width() - sz.width();
    qreal h = vsz.height() - sz.height();

    if ( w > 0 ) {
        m_extraPos.setX(w/2);
        gg.moveLeft(0);
    } else {
        m_extraPos.setX(0);
        if (gg.x() < w)
            gg.moveLeft(w);
        if (gg.x() > 0)
            gg.moveLeft(0);
    }

    if ( h > 0 ) {
        m_extraPos.setY(h/2);
        gg.moveTop(0);
    } else {
        m_extraPos.setY(0);
        if (gg.y() < h)
            gg.moveTop(h);
        if (gg.y() > 0)
            gg.moveTop(0);
    }
    gg.translate(m_extraPos);
    gg.translate(m_overShootDelta);
    return gg;
}

void PannableViewport::setPannedWidgetGeometry(const QRectF& g)
{
    pannedWidget()->setGeometry(adjustRectForPannedWidgetGeometry(g));
    updateScrollbars();
}

void PannableViewport::startPannedWidgetGeomAnim(const QRectF& geom)
{
    m_geomAnim.setStartValue(pannedWidget()->geometry());
    m_geomAnimEndValue = adjustRectForPannedWidgetGeometry(geom);
    m_geomAnim.setEndValue(m_geomAnimEndValue);
    m_geomAnim.start();
}

void PannableViewport::stopPannedWidgetGeomAnim()
{
    m_geomAnimEndValue = QRectF();
    m_geomAnim.stop();
}

void PannableViewport::transferAnimStateToView()
{
    if (m_geomAnimEndValue.isValid())
        pannedWidget()->setGeometry(m_geomAnimEndValue);
    updateScrollbars();
}

void PannableViewport::geomAnimStateChanged(QAbstractAnimation::State newState,QAbstractAnimation::State)
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
