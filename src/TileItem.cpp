#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsSimpleTextItem>
#include <QtGlobal>
#include <QTimer>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

#include "TileItem.h"
#include "UrlItem.h"

const int s_padding = 10;

TileItem::TileItem(QGraphicsWidget* parent, UrlItem& urlItem, TileLayout layout)
    : QGraphicsRectItem(parent)
    , m_urlItem(&urlItem)
    , m_layout(layout)
    , m_selected(false)
    , m_defaultIcon(0)
    , m_closeIcon(0)
    , m_dclick(false)
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
    delete m_closeIcon;
}

void TileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    QFont f("Times", 10);
    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);
    QRectF r(rect()); r.adjust(s_padding, s_padding, -s_padding, -s_padding);

    // QGraphicsDropShadowEffect doesnt perform well on n900.
    addDropShadow(*painter, r);

    painter->setBrush(m_bckgGradient);
    painter->setPen(Qt::black);
    painter->drawRoundedRect(r, 5, 5);
    QImage* image = m_defaultIcon ? m_defaultIcon : m_urlItem->thumbnail();
    painter->drawImage(m_thumbnailRect, *image, image->rect());

    painter->setFont(f);
    painter->setPen(QColor(0, 0, 0));
    painter->drawText(m_textRect, Qt::AlignHCenter|(m_layout == Horizontal ? Qt::AlignVCenter : Qt::AlignBottom), m_title);

    if (m_closeIcon)
        painter->drawImage(m_closeIconRect, *m_closeIcon, m_closeIcon->rect());

    if (m_selected) {
        painter->setBrush(QColor(80, 80, 80, 160));
        painter->drawRoundRect(r, 5, 5);
    }
}

void TileItem::setGeometry(const QRectF& rect)
{
    QRectF oldRect(geometry());
    setRect(rect);

    QRectF r(rect); r.adjust(s_padding, s_padding, -s_padding, -s_padding);

    if (!m_urlItem)    
        return;
    // reposition items
    QFont f("Times", 10);
    
    m_title = "";
    m_thumbnailRect = r;
    m_textRect = r;

    QImage* tn = m_urlItem->thumbnail();
    if (!tn)
        tn = m_defaultIcon;

    if (m_layout == Horizontal) {
        // thumbnail comes in front of the text, middle positioned
        m_thumbnailRect.moveTop(r.center().y() - tn->rect().height()/2);
        m_thumbnailRect.moveLeft(m_thumbnailRect.left() + 2);
        m_thumbnailRect.setSize(tn->rect().size());
        // fix center positioning
        m_textRect.adjust(m_thumbnailRect.width(), 0, -m_thumbnailRect.width(), 0);
    } else if (m_layout == Vertical) {
        if (m_defaultIcon) {
            m_thumbnailRect.setSize(m_defaultIcon->rect().size());
            m_thumbnailRect.moveCenter(r.center());
        } else {
            // stretch thumbnail
            m_thumbnailRect.adjust(2, 2, -2, -(QFontMetrics(f).height() + 3));
        }
    }
    
    setEditIconRect();

    m_bckgGradient.setStart(r.topLeft());
    m_bckgGradient.setFinalStop(r.bottomLeft());
    if (oldRect.size() != rect.size())
        m_title = QFontMetrics(f).elidedText(m_urlItem->m_title, Qt::ElideRight, m_textRect.width());
}

void TileItem::setEditMode(bool on) 
{ 
    if (on && !m_closeIcon) {
        m_closeIcon = new QImage(":/data/icon/48x48/close_item_48.png");
        setEditIconRect();
    } else if (!on) {
        delete m_closeIcon;
        m_closeIcon = 0;
    }
}

void TileItem::thumbnailChanged() 
{ 
    // new thumbnail? 
    if (m_defaultIcon && m_urlItem->thumbnailAvailable()) {
        delete m_defaultIcon;
        m_defaultIcon = 0;
    }
    update(); 
}

void TileItem::setEditIconRect()
{
    if (!m_closeIcon)
        return;
    m_closeIconRect = QRectF(rect().topRight(), m_closeIcon->rect().size());
    m_closeIconRect.moveRight(rect().right() + 5);
    m_closeIconRect.moveTop(rect().top() - 5);
}

void TileItem::addDropShadow(QPainter& painter, const QRectF rect)
{
    // FIXME: dropshadow shouldnt be a rect
    painter.setPen(QColor(40, 40, 40));
    painter.setBrush(QColor(20, 20, 20));
    QRectF r(rect); r.moveTopLeft(r.topLeft() + QPointF(2,2));
    painter.drawRoundedRect(r, 5, 5);
}

void TileItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_dclick)
        return;

    if (m_closeIcon && m_closeIconRect.contains(event->pos())) {
        emit itemClosed(m_urlItem);
    } else {    
        m_selected = true;
        emit itemActivated(m_urlItem);
    }
}

void TileItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    // half second timeout to distingush double click from click.
    // FIXME: this should be managed very differently. need to look at the gesture consumer
    m_dclick = true;
    QTimer::singleShot(500, this, SLOT(invalidateClick()));
    emit itemEditingMode(m_urlItem);
}

void TileItem::invalidateClick()
{
    m_dclick = false;
}

