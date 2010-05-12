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

const int s_hTextMargin = 10;

TileItem::TileItem(QGraphicsWidget* parent, UrlItem& urlItem, bool editable)
    : QGraphicsRectItem(parent)
    , m_urlItem(&urlItem)
    , m_selected(false)
    , m_closeIcon(0)
    , m_dclick(false)
    , m_editable(editable)
    , m_context(0)
    , m_dirty(true)
{
}

TileItem::~TileItem()
{
    delete m_closeIcon;
}

void TileItem::resizeEvent(QGraphicsSceneResizeEvent* /*event*/)
{
    m_dirty = true;
}

void TileItem::setEditMode(bool on) 
{ 
    if (!m_editable)
        return;
    if (on && !m_closeIcon) {
        m_closeIcon = new QImage(":/data/icon/48x48/close_item_48.png");
        setEditIconRect();
    } else if (!on) {
        delete m_closeIcon;
        m_closeIcon = 0;
    }
}

void TileItem::setEditIconRect()
{
    if (!m_closeIcon)
        return;
    m_closeIconRect = QRectF(rect().topRight(), m_closeIcon->rect().size());
    m_closeIconRect.moveRight(m_closeIconRect.right() - m_closeIconRect.width() + 10);
    m_closeIconRect.moveTop(m_closeIconRect.top() - 5);
}

void TileItem::paintExtra(QPainter* painter)
{
    if (m_closeIcon)
        painter->drawImage(m_closeIconRect, *m_closeIcon);

    if (m_selected) {
        painter->setBrush(QColor(80, 80, 80, 160));
        painter->drawRoundRect(rect(), 5, 5);
    }
}

void TileItem::addDropShadow(QPainter& painter, const QRectF rect)
{
    // FIXME: dropshadow shouldnt be a rect
    painter.setPen(QColor(40, 40, 40));
    painter.setBrush(QColor(20, 20, 20));
    QRectF r(rect); r.moveTopLeft(r.topLeft() + QPointF(2,2));
    painter.drawRoundedRect(r, 5, 5);
}

void TileItem::layoutTile()
{
    if (!m_dirty)
        return;
    doLayoutTile();
    setEditIconRect();
    m_dirty = false;
}

void TileItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*event*/)
{
}

void TileItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_dclick)
        return;

    // expand it to fit thumbs
    QRectF r(m_closeIconRect);
    r.adjust(-20, -20, 20, 20);
    if (m_closeIcon && r.contains(event->pos())) {
        emit itemClosed(this);
    } else {    
        m_selected = true;
        update();
        emit itemActivated(this);
    }
}

void TileItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    // half second timeout to distingush double click from click.
    // FIXME: this should be managed very differently. need to look at the gesture consumer
    m_dclick = true;
    QTimer::singleShot(500, this, SLOT(invalidateClick()));
    emit itemEditingMode(this);
}

void TileItem::invalidateClick()
{
    m_dclick = false;
}

////////////////////////////////////////////////////////////////////////////////
ThumbnailTileItem::ThumbnailTileItem(QGraphicsWidget* parent, UrlItem& urlItem, bool editable)
    : TileItem(parent, urlItem, editable)
    , m_defaultIcon(0)
{
    connect(m_urlItem, SIGNAL(thumbnailChanged()), this, SLOT(thumbnailChanged()));
    if (!urlItem.thumbnail())
        m_defaultIcon = new QImage(":/data/icon/48x48/defaulticon_48.png");
}

ThumbnailTileItem::~ThumbnailTileItem()
{
    delete m_defaultIcon;
}

void ThumbnailTileItem::thumbnailChanged() 
{ 
    // new thumbnail? 
    if (m_defaultIcon && m_urlItem->thumbnailAvailable()) {
        delete m_defaultIcon;
        m_defaultIcon = 0;
    }
    update(); 
}

void ThumbnailTileItem::doLayoutTile()
{
    QFont f("Times", 10);
    QRectF r(rect()); 

    m_textRect = r;
    m_thumbnailRect = r;

    if (m_defaultIcon) {
        m_thumbnailRect.setSize(m_defaultIcon->rect().size());
        m_thumbnailRect.moveCenter(r.center());
    } else {
        // stretch thumbnail
        m_thumbnailRect.adjust(2, 2, -2, -(QFontMetrics(f).height() + 3));
    }
    m_title = QFontMetrics(f).elidedText(m_urlItem->m_title, Qt::ElideRight, m_textRect.width());
}

void ThumbnailTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    QRectF r(rect()); 

    // QGraphicsDropShadowEffect doesnt perform well on n900.
    addDropShadow(*painter, r);
 
    painter->setBrush(Qt::white);
    painter->setPen(Qt::black);
    painter->drawRoundedRect(r, 5, 5);
    QRectF thumbnailRect(r);
    painter->drawImage(m_thumbnailRect, *(m_defaultIcon ? m_defaultIcon : m_urlItem->thumbnail()));

    painter->setFont(QFont("Times", 10));
    painter->setPen(Qt::black);
    painter->drawText(m_textRect, Qt::AlignHCenter|Qt::AlignBottom, m_title);
    paintExtra(painter);
}

NewWindowTileItem::NewWindowTileItem(QGraphicsWidget* parent, UrlItem& item)
    : ThumbnailTileItem(parent, item, false)

{
}

void NewWindowTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    painter->setPen(Qt::white);
    painter->setBrush(Qt::black);
    painter->drawRoundedRect(rect(), 5, 5);

    painter->setFont(QFont("Times", 12));
    painter->setPen(Qt::white);
    painter->drawText(rect(), Qt::AlignCenter, "Open new window");
}

NewWindowMarkerTileItem::NewWindowMarkerTileItem(QGraphicsWidget* parent, UrlItem& item)
    : ThumbnailTileItem(parent, item, false)

{
}

void NewWindowMarkerTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    QPen p(Qt::DashLine);
    p.setColor(QColor(100, 100, 100));
    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(rect(), 5, 5);
}

//
ListTileItem::ListTileItem(QGraphicsWidget* parent, UrlItem& urlItem, bool editable)
    : TileItem(parent, urlItem, editable)
{
}

void ListTileItem::doLayoutTile()
{
    QRectF r(rect()); 
    r.adjust(s_hTextMargin, 0, -s_hTextMargin, 0);
    m_titleRect = r;
    m_urlRect = r;

    QFont fbig("Times", 18);
    QFont fsmall("Times", 12);
    QFontMetrics fmfbig(fbig);
    QFontMetrics fmfsmall(fsmall);

    int fontHeightRatio = r.height() / (fmfbig.height() + fmfsmall.height() + 5);
    m_titleRect.setHeight(fmfbig.height() * fontHeightRatio);
    m_urlRect.setTop(m_titleRect.bottom() + 5); 

    m_title = fmfbig.elidedText(m_urlItem->m_title, Qt::ElideRight, r.width());
    m_url = fmfsmall.elidedText(m_urlItem->m_url.toString(), Qt::ElideRight, r.width());
}

void ListTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    QRectF r(rect()); 

    // QGraphicsDropShadowEffect doesnt perform well on n900.
    addDropShadow(*painter, r);
 
    painter->setBrush(Qt::white);
    painter->setPen(Qt::black);
    painter->drawRoundedRect(r, 3, 3);

    painter->setPen(Qt::black);
    painter->setFont(QFont("Times", 18));
    painter->drawText(m_titleRect, Qt::AlignLeft|Qt::AlignVCenter, m_title);

    painter->setPen(QColor(110, 110, 110));
    painter->setFont(QFont("Times", 12));
    painter->drawText(m_urlRect, Qt::AlignLeft|Qt::AlignVCenter, m_url);

    paintExtra(painter);
}


