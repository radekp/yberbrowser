#include "TileSelectionViewBase.h"
#include <QGraphicsRectItem>
#include <QTimer>

#include "ApplicationWindow.h"
#include "TileItem.h"

TileSelectionViewBase::TileSelectionViewBase(ViewType type, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_bckg(new QGraphicsRectItem(this))
    , m_type(type)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);

    m_bckg->setBrush(QColor(20, 20, 20));
    m_bckg->setZValue(0);
//    m_bckg->setOpacity(0.8);
}

TileSelectionViewBase::~TileSelectionViewBase()
{
    delete m_bckg;
}

void TileSelectionViewBase::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    m_bckg->setRect(rect());
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

