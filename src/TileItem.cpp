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
    , m_selected(false)
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

    // QGraphicsDropShadowEffect doesnt perform well on n900.
    if (textOnly) {
        addDropShadow(*painter, rect());

        painter->setPen(QColor(140, 140, 140));
        painter->setBrush(QColor(240, 240, 240));
        painter->drawRoundedRect(rect(), 5, 5);
    } else {    
        addDropShadow(*painter, m_thumbnailRect);
        painter->drawImage(m_thumbnailRect, *m_urlItem->thumbnail(), m_urlItem->thumbnail()->rect());
    }

    painter->setFont(f);
    painter->setPen(textOnly ? QColor(22, 22, 22) : QColor(220, 220, 220));
    painter->drawText(rect(), Qt::AlignHCenter|(textOnly ? Qt::AlignVCenter : Qt::AlignBottom), m_title);

    if (m_selected) {
        painter->setBrush(QColor(80, 80, 80, 160));
        painter->drawRoundRect(rect(), 5, 5);
    }
}

void TileItem::setGeometry(const QRectF& rect)
{
    setRect(rect);
    
    // reposition items
    m_title = "";
    if (m_urlItem) {
        QFont f("Times", 10);
        m_title = QFontMetrics(f).elidedText(m_urlItem->m_title, Qt::ElideRight, rect.width());
    }
    
    if (!m_textOnly) {
        m_thumbnailRect = rect.toRect();
        m_thumbnailRect.setHeight(m_thumbnailRect.height() - (QFontMetrics(QFont()).height() + 3));
    }
}

void TileItem::addDropShadow(QPainter& painter, const QRectF rect)
{
    painter.setPen(QColor(40, 40, 40));
    painter.setBrush(QColor(20, 20, 20));
    QRectF r(rect); r.moveTopLeft(r.topLeft() + QPointF(2,2));
    painter.drawRoundedRect(r, 5, 5);
}

void TileItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    m_selected = true;
    emit itemActivated(m_urlItem);
}

