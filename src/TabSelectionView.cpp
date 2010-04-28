#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>

#include "TabSelectionView.h"
#include "TileContainerWidget.h"
#include "TileItem.h"
#include "HistoryStore.h"
#include "BookmarkStore.h"
#include "ApplicationWindow.h"

#include <QPropertyAnimation>
#if USE_DUI
#include <DuiScene>
#endif

// FIXME: this is a complete fixme class

namespace {
const int s_tilePadding = 20;
}

class TabWidget : public TileBaseWidget {
    Q_OBJECT
public:
    TabWidget(QGraphicsItem*, Qt::WindowFlags wFlags = 0);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void setupWidgetContent();
};

TabWidget::TabWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileBaseWidget(HistoryStore::instance()->list(), parent, wFlags)
{
}

void TabWidget::setupWidgetContent()
{
    int width = parentWidget()->geometry().width();

    // default 
    int hTileNum = m_urlList->size();
    int tileWidth = (width / 2) - s_tilePadding;
    int tileHeight = tileWidth / 1.20;
    
    QRectF r(rect());
    r.setWidth((tileWidth + s_tilePadding) * hTileNum);
    setGeometry(r);

    // move tiles to the middle
    r.setTop(r.center().y() - (tileHeight / 2));
    r.setHeight(tileHeight);

    addTiles(r, hTileNum, tileWidth, 1, tileHeight, s_tilePadding, 0, TileItem::Vertical);
}

void TabWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    TileBaseWidget::paint(painter, option, widget);
}

TabSelectionView::TabSelectionView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_pannableTabContainer(new PannableTileContainer(this, wFlags))
    , m_tabWidget(new TabWidget(this, wFlags))
{
    m_tabWidget->setZValue(1);
    m_pannableTabContainer->setWidget(m_tabWidget);
    connect(m_tabWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

TabSelectionView::~TabSelectionView()
{
    delete m_tabWidget;
    delete m_pannableTabContainer;
}

void TabSelectionView::setGeometry(const QRectF& rect)
{
    QRectF currentRect(geometry());
    if (rect == currentRect)
        return;
    
    if (rect.width() != currentRect.width() || rect.height() != currentRect.height()) {
        // readjust subcontainers' sizes in case of size chage
        m_pannableTabContainer->setGeometry(rect);
        m_tabWidget->setGeometry(rect);
    }
    TileSelectionViewBase::setGeometry(rect);
}

void TabSelectionView::tileItemActivated(UrlItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);
}

void TabSelectionView::setupAnimation(bool in)
{
    // add both history and bookmark anim
    QPropertyAnimation* tabAnim = new QPropertyAnimation(m_tabWidget, "geometry");
    tabAnim->setDuration(800);
    QRectF r(m_tabWidget->geometry());

    if (in)
        r.moveLeft(rect().left());
    QRectF hiddenWidget(r); hiddenWidget.moveLeft(r.right());

    tabAnim->setStartValue(in ?  hiddenWidget : r);
    tabAnim->setEndValue(in ? r : hiddenWidget);

    tabAnim->setEasingCurve(in ? QEasingCurve::OutBack : QEasingCurve::InCubic);

    m_slideAnimationGroup->addAnimation(tabAnim);

    // hide the container
    if (in)
        m_tabWidget->setGeometry(hiddenWidget);
}

void TabSelectionView::destroyViewItems()
{
    m_tabWidget->destroyWidgetContent();
}

void TabSelectionView::createViewItems()
{
    m_tabWidget->setupWidgetContent();
}

#include "TabSelectionView.moc"
