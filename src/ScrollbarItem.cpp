#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QPropertyAnimation>

#include "ScrollbarItem.h"

#include <QDebug>


static const qreal s_thumbSize = 6;
static const qreal s_thumbMinSize = 20;
static const qreal s_thumbMargin = 12;

const unsigned s_scrollbarFadeTimeout = 300; // in msec
const unsigned s_scrollbarFadeDuration = 300; // in msec

const qreal s_scrollbarOpacityStart = 0.2;
const qreal s_scrollbarOpacity = 0.8;

ScrollbarItem::ScrollbarItem(Qt::Orientation orientation, QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , m_orientation(orientation)
    , m_fadeAnim(this, "opacity")
    , m_fadeOutTimeout(this)
{
    m_fadeAnim.setDuration(s_scrollbarFadeDuration);
    m_fadeAnim.setStartValue(s_scrollbarOpacityStart);
    m_fadeAnim.setEndValue(s_scrollbarOpacity);
    m_fadeAnim.setEasingCurve(QEasingCurve::Linear);
    connect(&m_fadeAnim, SIGNAL(finished()), this, SLOT(fadingFinished()));

    m_fadeOutTimeout.setInterval(s_scrollbarFadeTimeout);
    m_fadeOutTimeout.setSingleShot(true);
    connect(&m_fadeOutTimeout, SIGNAL(timeout()), this, SLOT(startFadeOut()));
}

ScrollbarItem::~ScrollbarItem()
{
}

void ScrollbarItem::updateVisibilityAndFading(bool shouldFadeOut)
{
    show();

    if (opacity() != s_scrollbarOpacity)
        startFading(true);

    m_fadeOutTimeout.stop();
    if (shouldFadeOut)
        m_fadeOutTimeout.start();
}

void ScrollbarItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(QBrush(QColor(60, 60, 60)), 1));
    painter->setBrush(QColor(100, 100, 100));
    painter->drawRoundRect(rect(), 10, 10);
}

void ScrollbarItem::fadingFinished()
{
    // hide when fading out
    if (opacity() != s_scrollbarOpacity)
        hide();
}

void ScrollbarItem::startFadeOut()
{
    startFading(false);
}

void ScrollbarItem::startFading(bool in)
{
    if (in) {
        m_fadeAnim.setDirection(QAbstractAnimation::Forward);

    } else {
        m_fadeAnim.setDirection(QAbstractAnimation::Backward);
    }

    if (m_fadeAnim.state() != QAbstractAnimation::Running)
        m_fadeAnim.start();
}

/*!
  \a contentPos includes overshoot
*/

void ScrollbarItem::contentPositionUpdated(qreal contentPos, qreal contentLength, const QSizeF& viewSize, bool shouldFadeOut)
{
    qreal viewLength = m_orientation == Qt::Horizontal ? viewSize.width() : viewSize.height();

    if (contentLength < viewLength)
        contentLength = viewLength;

    qreal thumbRange = viewLength - 2 * s_thumbMargin;

    qreal thumbPos = (thumbRange) * (-contentPos  / (contentLength));
    qreal thumbPosMax = (thumbRange) * (-contentPos + viewLength)  / (contentLength);

    thumbPos = qBound(static_cast<qreal>(0.), thumbPos, thumbRange - s_thumbMinSize);
    thumbPosMax = qBound(s_thumbMinSize, thumbPosMax, thumbRange);
    qreal thumbLength = thumbPosMax - thumbPos;

    // FIXME: What's the number 5 below?
    if (m_orientation == Qt::Horizontal)
        setRect(QRectF(s_thumbMargin + thumbPos, viewSize.height() - s_thumbSize - 5, thumbLength, s_thumbSize));
    else
        setRect(QRectF(viewSize.width() - s_thumbSize - 5, s_thumbMargin + thumbPos, s_thumbSize,  thumbLength));

    updateVisibilityAndFading(shouldFadeOut);
}
