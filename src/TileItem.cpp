#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsSimpleTextItem>
#include <QtGlobal>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

#include "TileItem.h"
#include "UrlStore.h"
#include "UrlItem.h"

TileItem::TileItem(QGraphicsWidget* parent, UrlItem* urlItem, bool textOnly)
    : QGraphicsRectItem(parent)
    , m_urlItem(urlItem)
    , m_textOnly(textOnly)
{
    if (m_urlItem)
        connect(m_urlItem, SIGNAL(thumbnailChanged()), this, SLOT(thumbnailChanged()));
}

TileItem::~TileItem()
{
}

void TileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    bool textOnly = !m_urlItem || (m_urlItem && !m_urlItem->thumbnail()) || m_textOnly;
    QFont f("Times", 10);
    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    if (textOnly) {
        painter->setPen(QColor(240, 240, 240));
        painter->setBrush(QColor(140, 140, 140));
        painter->drawRoundedRect(rect(), 5, 5);
    } else {    
        painter->drawImage(m_thumbnailRect, *m_urlItem->thumbnail(), m_urlItem->thumbnail()->rect());
    }

    painter->setFont(f);
    painter->setPen(textOnly ? QColor(22, 22, 22) : QColor(220, 220, 220));
    painter->drawText(rect(), Qt::AlignHCenter|(textOnly ? Qt::AlignVCenter : Qt::AlignBottom), m_title);
}

void TileItem::setGeometry(const QRectF& rect)
{
    setRect(rect);
    
    // reposition items
    m_title = "";
    if (m_urlItem)
        m_title = QFontMetrics(QFont()).elidedText(m_urlItem->m_title, Qt::ElideRight, rect.width());
    
    if (!m_textOnly) {
        m_thumbnailRect = rect.toRect();
        m_thumbnailRect.setHeight(m_thumbnailRect.height() - (QFontMetrics(QFont()).height() + 3));
    }
}

void TileItem::addDropshadow()
{
    if (graphicsEffect())
        setGraphicsEffect(0);

    QGraphicsDropShadowEffect* d = new QGraphicsDropShadowEffect(this);
    d->setColor(QColor(20, 20, 20));
    d->setOffset(QPoint(2, 2));
    d->setBlurRadius(5);
    setGraphicsEffect(d);
    update();
}

void TileItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    emit itemActivated(m_urlItem);
}

