#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsSimpleTextItem>
#include <QGLWidget>
#include <QtGlobal>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

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

    painter->setPen(QColor(220, 220, 220));
    painter->drawText(rect(), Qt::AlignHCenter|Qt::AlignBottom, m_title);

    if (!m_urlItem || (m_urlItem && !m_urlItem->thumbnail()))
        return;

    painter->drawImage(m_thumbnailRect, *m_urlItem->thumbnail(), m_urlItem->thumbnail()->rect());
}

void HistoryItem::setGeometry(const QRectF& rect)
{
    setRect(rect);
    
    // reposition items
    m_title = "";
    if (m_urlItem)
        m_title = QFontMetrics(QFont()).elidedText(m_urlItem->m_title, Qt::ElideRight, rect.width());
        
    m_thumbnailRect = rect.toRect();
    m_thumbnailRect.setHeight(m_thumbnailRect.height() - (QFontMetrics(QFont()).height() + 3));
}

void HistoryItem::addDropshadow()
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

void HistoryItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    emit itemActivated(m_urlItem);
}

