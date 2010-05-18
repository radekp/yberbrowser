#include "TileSelectionViewBase.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QTimer>

#include "ApplicationWindow.h"
#include "TileItem.h"

TileSelectionViewBase::TileSelectionViewBase(ViewType type, QPixmap* bckg, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_bckg(new QGraphicsPixmapItem(*bckg, this))
    , m_type(type)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
}

TileSelectionViewBase::~TileSelectionViewBase()
{
    delete m_bckg;
}

void TileSelectionViewBase::setGeometry( const QRectF &r)
{
    QGraphicsWidget::setGeometry(r);
    m_bckg->setPos(-geometry().topLeft());
}

void TileSelectionViewBase::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    m_bckg->setPos(-pos());
    createViewItems();
    QGraphicsWidget::resizeEvent(event);
}

void TileSelectionViewBase::appear(ApplicationWindow* window)
{
    // FIXME: how to test if view is already in correct view?
    if (!scene()) {
        window->scene()->addItem(this);
        setZValue(100);
    }
    scene()->setActiveWindow(this);
    createViewItems();
    // bckg pos is misbehaving on device (n900), need to do an extra setPos here
    m_bckg->setPos(-pos());
    emit appeared();
}

void TileSelectionViewBase::disappear()
{
    destroyViewItems();
    // FIXME: what's wrong with sync emit disappeared
    QTimer::singleShot(0, this, SLOT(deleteView()));
}

void TileSelectionViewBase::tileItemActivated(TileItem* /*item*/)
{
    disappear();
}

void TileSelectionViewBase::deleteView()
{
    emit disappeared(this);
}

void TileSelectionViewBase::connectItem(TileItem& item)
{
    connect(&item, SIGNAL(itemActivated(TileItem*)), this, SLOT(tileItemActivated(TileItem*)));
    connect(&item, SIGNAL(itemClosed(TileItem*)), this, SLOT(tileItemClosed(TileItem*)));
    connect(&item, SIGNAL(itemEditingMode(TileItem*)), this, SLOT(tileItemEditingMode(TileItem*)));
}

