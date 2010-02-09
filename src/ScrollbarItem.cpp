#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QPropertyAnimation>

#include "ScrollbarItem.h"

#include <QDebug>

const unsigned s_scrollbarTimeout = 300; // in msec
const qreal s_scrollbarOpacity = 0.8;

ScrollbarItem::ScrollbarItem(QGraphicsItem* parent, bool horizontal)
    : QGraphicsRectItem(parent)
    , m_visibilityTimer(this)
    , m_horizontal(horizontal)
    , m_fader(new QPropertyAnimation(this, "opacity"))
{
    hide();
    m_visibilityTimer.setSingleShot(true);
    connect(&m_visibilityTimer, SIGNAL(timeout()), this, SLOT(fadeScrollbar()));
    connect(m_fader, SIGNAL(finished()), this, SLOT(fadingFinished()));
}

ScrollbarItem::~ScrollbarItem()
{
    delete m_fader;
}

void ScrollbarItem::show()
{
    // not visible or fading out
    bool needsFading = (!isVisible() || (!m_visibilityTimer.isActive() && isVisible()));

    if (m_visibilityTimer.isActive())
        m_visibilityTimer.stop();

    if (!isVisible())
        QGraphicsRectItem::show();

    if (needsFading)
        startFading(true);

    m_visibilityTimer.start(s_scrollbarTimeout);
}

void ScrollbarItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(QBrush(QColor(60, 60, 60)), 1));
    painter->setBrush(QColor(100, 100, 100));
    painter->drawRoundRect(rect(), 10, 10);
}

void ScrollbarItem::fadeScrollbar()
{
    startFading(false);
}

void ScrollbarItem::fadingFinished()
{
    // hide when fading out
    if (opacity() != s_scrollbarOpacity)
        hide();
}

void ScrollbarItem::startFading(bool in)
{
    m_fader->setDuration(300);

    m_fader->setStartValue(in ? 0.2 : s_scrollbarOpacity);
    m_fader->setEndValue(in ? s_scrollbarOpacity : 0.2);    

    m_fader->setEasingCurve(QEasingCurve::Linear);
    m_fader->start();
}

