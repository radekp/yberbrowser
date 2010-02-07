#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsSimpleTextItem>
#include <QGLWidget>
#include <QtGlobal>
#include <QDebug>

#include "HistoryItem.h"
#include "MainWindow.h"
#include "UrlStore.h"
#include "UrlItem.h"

HistoryItem::HistoryItem(QGraphicsWidget* parent, UrlItem* urlItem)
    : QGraphicsRectItem(parent)
    , m_urlItem(urlItem)
{
    if (m_urlItem)
        connect(m_urlItem, SIGNAL(thumbnailChanged()), this, SLOT(thumbnailChanged()));
}

HistoryItem::~HistoryItem()
{
}

void HistoryItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    painter->setPen(QPen(QBrush(QColor(109, 109, 109)), 3));
    painter->setBrush(QColor(139, 139, 139));
    painter->drawRect(m_thumbnailRect);
    
    painter->setPen(QColor(220, 220, 220));
    painter->drawText(m_titlePos, m_title);
    
    if (!m_urlItem || (m_urlItem && !m_urlItem->thumbnail()))
        return;

    QRectF r(QPoint(m_thumbnailRect.topLeft() + QPoint(3,3)), QSize(m_thumbnailRect.size() - QSize(6,6)));
    painter->drawImage(r, *m_urlItem->thumbnail(), m_urlItem->thumbnail()->rect());
}

void HistoryItem::setGeometry(const QRectF& rect)
{
    setRect(rect);
    
    // reposition items
    m_title = "";
    if (m_urlItem)
        m_title = m_urlItem->m_title;
        
    m_title = QFontMetrics(QFont()).elidedText(m_title, Qt::ElideRight, rect.width());
    QSize textSize = QFontMetrics(QFont()).size(Qt::TextSingleLine, m_title);
    m_thumbnailRect = rect.toRect();
    m_thumbnailRect.setHeight(m_thumbnailRect.height() - textSize.height());

    m_titlePos = QPoint(m_thumbnailRect.left() + ((m_thumbnailRect.width() - textSize.width()) / 2), m_thumbnailRect.bottom() + QFontMetrics(QFont()).height() + 2);
}

void HistoryItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    emit itemActivated(m_urlItem);
}

#include "HistoryItem.moc"
