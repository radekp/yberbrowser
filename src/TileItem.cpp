#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsSimpleTextItem>
#include <QtGlobal>
#include <QDebug>

#include "TileItem.h"
#include "UrlItem.h"

TileItem::TileItem(QGraphicsWidget* parent, UrlItem& urlItem, TileLayout layout)
    : QGraphicsRectItem(parent)
    , m_urlItem(&urlItem)
    , m_layout(layout)
    , m_selected(false)
    , m_defaultIcon(0)
{
    connect(m_urlItem, SIGNAL(thumbnailChanged()), this, SLOT(thumbnailChanged()));
    if (!m_urlItem->thumbnail())
        m_defaultIcon = new QImage(layout == Horizontal ? ":/data/icon/16x16/defaulticon_16.png" : ":/data/icon/48x48/defaulticon_48.png");
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(255, 255, 255)) << QGradientStop(0.10, QColor(245, 245, 245)) << QGradientStop(0.50, QColor(214, 214, 214)) << QGradientStop(0.90, QColor(245, 245, 245)) << QGradientStop(1.00, QColor(255, 255, 255));
    for (int j=0; j<stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);
}

TileItem::~TileItem()
{
    delete m_defaultIcon;
}

void TileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    QFont f("Times", 10);
    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    // QGraphicsDropShadowEffect doesnt perform well on n900.
    addDropShadow(*painter, rect());

    painter->setBrush(m_bckgGradient);
    painter->setPen(Qt::black);
    painter->drawRoundedRect(rect(), 5, 5);
    QImage* image = m_defaultIcon ? m_defaultIcon : m_urlItem->thumbnail();
    painter->drawImage(m_thumbnailRect, *image, image->rect());

    painter->setFont(f);
    painter->setPen(QColor(0, 0, 0));
    painter->drawText(m_textRect, Qt::AlignHCenter|(m_layout == Horizontal ? Qt::AlignVCenter : Qt::AlignBottom), m_title);

    if (m_selected) {
        painter->setBrush(QColor(80, 80, 80, 160));
        painter->drawRoundRect(rect(), 5, 5);
    }
}

void TileItem::setGeometry(const QRectF& rect)
{
    setRect(rect);

    if (!m_urlItem)    
        return;
    // reposition items
    QFont f("Times", 10);
    
    m_title = "";
    m_thumbnailRect = rect;
    m_textRect = rect;

    QImage* tn = m_urlItem->thumbnail();
    if (!tn)
        tn = m_defaultIcon;

    if (m_layout == Horizontal) {
        // thumbnail comes in front of the text, middle positioned
        m_thumbnailRect.moveTop(rect.center().y() - tn->rect().height()/2);
        m_thumbnailRect.moveLeft(m_thumbnailRect.left() + 2);
        m_thumbnailRect.setSize(tn->rect().size());
        // fix center positioning
        m_textRect.adjust(m_thumbnailRect.width(), 0, -m_thumbnailRect.width(), 0);
    } else if (m_layout == Vertical) {
        if (m_defaultIcon) {
            m_thumbnailRect.setSize(m_defaultIcon->rect().size());
            m_thumbnailRect.moveCenter(rect.center());
        } else {
            // stretch thumbnail
            m_thumbnailRect.adjust(2, 2, -2, -(QFontMetrics(f).height() + 3));
        }
    }

    m_bckgGradient.setStart(rect.topLeft());
    m_bckgGradient.setFinalStop(rect.bottomLeft());
    m_title = QFontMetrics(f).elidedText(m_urlItem->m_title, Qt::ElideRight, m_textRect.width());
}

void TileItem::addDropShadow(QPainter& painter, const QRectF rect)
{
    // FIXME: dropshadow shouldnt be a rect
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

