#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsDropShadowEffect>
#include <QGLWidget>
#include <QtGlobal>
#include <QDebug>

#include "HistoryItem.h"
#include "PageView.h"
#include "MainWindow.h"
#include "UrlStore.h"
#include "UrlItem.h"

class ThumbnailGraphicsItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    ThumbnailGraphicsItem(QGraphicsWidget* parent, HistoryItem& historyItem);
    ~ThumbnailGraphicsItem();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void sizeChanged(const QRectF& rect);

Q_SIGNALS:
    void clicked();

public Q_SLOTS:
    void thumbnailChanged() { update(); }

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);

private:
    HistoryItem* m_historyItem;
    QRect m_thumbnailRect;
    QPoint m_titlePos;
    QString m_title;
};

ThumbnailGraphicsItem::ThumbnailGraphicsItem(QGraphicsWidget* parent, HistoryItem& historyItem)
    : QGraphicsRectItem(parent)
    , m_historyItem(&historyItem)
{
    if (m_historyItem->urlItem())
        connect(m_historyItem->urlItem(), SIGNAL(thumbnailChanged()), this, SLOT(thumbnailChanged()));
}

ThumbnailGraphicsItem::~ThumbnailGraphicsItem()
{
}

void ThumbnailGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);

    painter->setPen(QPen(QBrush(QColor(109, 109, 109)), 3));
    painter->setBrush(QColor(139, 139, 139));
    painter->drawRect(m_thumbnailRect);
    
    painter->setPen(QColor(220, 220, 220));
    painter->drawText(m_titlePos, m_title);
    
    if (!m_historyItem->urlItem() || (m_historyItem->urlItem() && !m_historyItem->urlItem()->thumbnail()))
        return;

    QRectF r(QPoint(m_thumbnailRect.topLeft() + QPoint(3,3)), QSize(m_thumbnailRect.size() - QSize(6,6)));
    painter->drawImage(r, *m_historyItem->urlItem()->thumbnail(), m_historyItem->urlItem()->thumbnail()->rect());
}

void ThumbnailGraphicsItem::sizeChanged(const QRectF& rect)
{
    setRect(rect);
    UrlItem* urlItem = m_historyItem->urlItem();
    
    // reposition items
    m_title = "";
    if (urlItem)
        m_title = urlItem->m_title;
        
    m_title = QFontMetrics(QFont()).elidedText(m_title, Qt::ElideRight, rect.width());
    QSize textSize = QFontMetrics(QFont()).size(Qt::TextSingleLine, m_title);
    m_thumbnailRect = rect.toRect();
    m_thumbnailRect.setHeight(m_thumbnailRect.height() - textSize.height());

    m_titlePos = QPoint(m_thumbnailRect.left() + ((m_thumbnailRect.width() - textSize.width()) / 2), m_thumbnailRect.bottom() + QFontMetrics(QFont()).height() + 2);
}

void ThumbnailGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    emit clicked();
}

HistoryItem::HistoryItem(QGraphicsWidget* parent, UrlItem* urlItem)
    : m_urlItem(urlItem)
{
    m_thumbnailRect = new ThumbnailGraphicsItem(parent, *this);
    connect(m_thumbnailRect, SIGNAL(clicked()), this, SLOT(thumbnailClicked()));
}

HistoryItem::~HistoryItem()
{
    delete m_thumbnailRect;
}

void HistoryItem::setPos(const QPointF& pos)
{
    m_thumbnailRect->setPos(pos);
}

QPointF HistoryItem::pos() const
{
    return m_thumbnailRect->pos();
}

QRectF HistoryItem::geometry() const
{
    return m_thumbnailRect->rect();
}

void HistoryItem::setGeometry(const QRectF& rect)
{
    m_thumbnailRect->sizeChanged(rect);
}

void HistoryItem::thumbnailClicked()
{
    if (m_urlItem)
        emit load(m_urlItem->m_url.toString());
}

#include "HistoryItem.moc"
