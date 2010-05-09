#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>

#include "HomeView.h"
#include "TileContainerWidget.h"
#include "TileItem.h"
#include "HistoryStore.h"
#include "BookmarkStore.h"
#include "ApplicationWindow.h"
#include <QGraphicsPixmapItem>
#include <QTimer>

#include <QPropertyAnimation>
#if USE_DUI
#include <DuiScene>
#endif

// FIXME: HomeView should be either a top level view, or just a central widget of the browsingview, probably the first one
// also, LAF is not finalized yet.

namespace {
const int s_minTileWidth = 170;
const int s_maxTileWidth = 200;
const int s_defaultTileNumH = 3;
const int s_defaultTileNumV = 6;
const int s_tileHistoryVMargin = 20;
const int s_tileHistoryHMargin = 10;
const int s_tileBookmarkPadding = 0;
const int s_bookmarksTileHeight = 70;

const int s_maxHistoryTileNum = s_defaultTileNumH * s_defaultTileNumV;
}

class HistoryWidget : public TileBaseWidget {
    Q_OBJECT
public:
    HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) :TileBaseWidget("Visited pages", parent, wFlags) {}
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    void layoutTiles();

private:
    void layoutHint(const QSizeF& constraint, int& hTileNum, int& vTileNum, QSizeF& tileSize) const;
};

QSizeF HistoryWidget::sizeHint (Qt::SizeHint /*which*/, const QSizeF & constraint) const
{
    int hTileNum;
    int vTileNum;
    QSizeF tileSize;
    QSizeF cSize(constraint);

    if (cSize.width() == -1)
        cSize.setWidth(parentWidget()->rect().size().width());
    layoutHint(cSize, hTileNum, vTileNum, tileSize);

    return QSizeF(hTileNum * (tileSize.width() + s_tileHistoryHMargin), vTileNum * (tileSize.height() + s_tileHistoryVMargin));
}

void HistoryWidget::layoutTiles()
{
    int hTileNum;
    int vTileNum;
    QSizeF tileSize;

    layoutHint(rect().size(), hTileNum, vTileNum, tileSize);

    // the height of the view is unknow until we figure out how many items there are
    QRectF r(rect()); 
    r.setHeight(vTileNum * (tileSize.height() + s_tileHistoryVMargin));
    setMinimumHeight(r.height());
    setMaximumWidth(hTileNum * (tileSize.width() + s_tileHistoryHMargin));
    doLayoutTiles(r, hTileNum, tileSize.width(), vTileNum, tileSize.height(), s_tileHistoryHMargin, s_tileHistoryVMargin);
}

void HistoryWidget::layoutHint(const QSizeF& constraint, int& hTileNum, int& vTileNum, QSizeF& tileSize) const
{
    // default 
    hTileNum = s_defaultTileNumH;
    int tileWidth = constraint.width() / hTileNum;

    // calculate the optimal tile width
    while (tileWidth < s_minTileWidth && hTileNum > 0)
        tileWidth = constraint.width() / --hTileNum;

    tileWidth = qMin(tileWidth, s_maxTileWidth);

    // keep height proposional
    vTileNum = s_defaultTileNumV;
    int tileHeight = tileWidth * 0.9;

    tileSize = QSize(tileWidth, tileHeight);
}

class BookmarkWidget : public TileBaseWidget {
    Q_OBJECT
public:
    BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) :TileBaseWidget("Bookmarks", parent, wFlags) {}

    void layoutTiles();
};

void BookmarkWidget::layoutTiles()
{
    // add bookmark tiles
    int vTileNum = m_tileList.size();
    int hTileNum = 1;
    int tileHeight = s_bookmarksTileHeight;

    QRectF r(rect());
    r.setHeight(tileHeight * m_tileList.size());
    // adjust the height of the view
    setMinimumHeight(r.height());
    setMaximumWidth(rect().width());
    // vertical adjustment
    r.adjust(10, 5, 0, -10);
    doLayoutTiles(r, hTileNum, r.width() - 60, vTileNum, tileHeight, 0, 0);
}

HomeView::HomeView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_bookmarkWidget(new BookmarkWidget(this, wFlags))
    , m_historyWidget(new HistoryWidget(this, wFlags))
    , m_pannableContainer(new PannableTileContainer(this, wFlags))
{
    setFiltersChildEvents(true);
    m_bookmarkWidget->setZValue(1);
    m_historyWidget->setZValue(1);

    m_pannableContainer->setHomeView(this);
    m_pannableContainer->setWidget(m_historyWidget);
    m_historyWidget->setActive(true);

    connect(m_bookmarkWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
    connect(m_historyWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

HomeView::~HomeView()
{
    delete m_bookmarkWidget;
    delete m_historyWidget;
    delete m_pannableContainer;
}

bool HomeView::sceneEventFilter(QGraphicsItem* /*i*/, QEvent* e)
{
    bool doFilter = false;

    if (!isVisible())
        return doFilter;

    switch (e->type()) {
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick:
        doFilter = recognizeFlick(static_cast<QGraphicsSceneMouseEvent *>(e));
        break;
    default:
        break;
    }
    return doFilter;
}

bool HomeView::recognizeFlick(QGraphicsSceneMouseEvent* e)
{
    if (e->type() == QEvent::GraphicsSceneMousePress) {
        m_mouseDown = true;
        m_mousePos = e->scenePos();
        m_hDelta = 0;
    } else if (e->type() == QEvent::GraphicsSceneMouseRelease) {
        m_mouseDown = false;
        // swap, move back, or no move at all?
        if (abs(m_hDelta) > 0) {
            moveViews(abs(m_hDelta) >= rect().width()/4);
            return true;
        }
    } if (e->type() == QEvent::GraphicsSceneMouseMove && m_mouseDown) {
        // panning
        QPointF delta(m_mousePos - e->scenePos());
        if (abs(delta.x()) >= abs(delta.y())) {
            bool move = (m_historyWidget->active() && (delta.x() > 0 || m_hDelta > 0)) 
                || (m_bookmarkWidget->active() && (delta.x() < 0 || m_hDelta < 0));

            if (!move)
                return true;

            QRectF rect(m_historyWidget->geometry());
            rect.moveLeft(rect.left() - delta.x());
            m_historyWidget->setGeometry(rect);

            rect = m_bookmarkWidget->geometry();
            rect.moveLeft(rect.left() - delta.x());
            m_bookmarkWidget->setGeometry(rect);
            m_mousePos = e->scenePos();
            m_hDelta+=delta.x();
            return true;
        }
    }
    return false;
}

void HomeView::setGeometry(const QRectF& rect)
{
    TileSelectionViewBase::setGeometry(rect);
    m_pannableContainer->setGeometry(rect);

    QRectF r(rect);
    // move history to the middle
    QSizeF s = m_historyWidget->sizeHint(Qt::PreferredSize, rect.size());
    r.moveLeft(rect.center().x() - (s.width()/2));
    r.setWidth(s.width());
    m_historyWidget->setGeometry(r);
    
    r = rect;
    r.moveLeft(rect.right() - 60);
    m_bookmarkWidget->setGeometry(r);
}

void HomeView::tileItemActivated(TileItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);

    if (!item || !item->urlItem())
        return;
    emit pageSelected(item->urlItem()->m_url);
}

void HomeView::tileItemClosed(TileItem* item)
{
    TileSelectionViewBase::tileItemClosed(item);
    if (m_bookmarkWidget->active())
        m_bookmarkWidget->removeTile(*item);
    else
        m_historyWidget->removeTile(*item);
}

void HomeView::tileItemEditingMode(TileItem* item)
{
    TileSelectionViewBase::tileItemEditingMode(item);
    if (m_bookmarkWidget->active())
        m_bookmarkWidget->setEditMode(!m_bookmarkWidget->editMode());
    else
        m_historyWidget->setEditMode(!m_historyWidget->editMode());
    update();
}

void HomeView::moveViews(bool swap)
{
    if (swap) {
        m_historyWidget->setActive(!m_historyWidget->active());
        m_bookmarkWidget->setActive(!m_bookmarkWidget->active());
        m_pannableContainer->detachWidget();
        if (m_historyWidget->active()) {
            m_bookmarkWidget->setParent(this);
            m_pannableContainer->setWidget(m_historyWidget);
        } else {
            m_historyWidget->setParent(this);
            m_pannableContainer->setWidget(m_bookmarkWidget);
        }
    }

    m_slideAnimationGroup->stop();
    m_slideAnimationGroup->clear();

    QPropertyAnimation* historyAnim = new QPropertyAnimation(m_historyWidget, "geometry");
    QPropertyAnimation* bookmarkAnim = new QPropertyAnimation(m_bookmarkWidget, "geometry");

    historyAnim->setDuration(800);
    bookmarkAnim->setDuration(800);

    QRectF from(m_historyWidget->geometry());
    QRectF to(m_historyWidget->geometry());
    if (m_historyWidget->active()) {
        // move history items back to the middle
        to.moveLeft(geometry().center().x() - (m_historyWidget->size().width()/2));
    } else {
        // move history items to the left edge, still visible a bit
        to.moveRight(geometry().left() + 30);
    }
        
    historyAnim->setStartValue(from);
    historyAnim->setEndValue(to);

    from = m_bookmarkWidget->geometry();
    to = m_bookmarkWidget->geometry();
    if (m_bookmarkWidget->active()) {
        to.moveLeft(geometry().center().x() - (m_bookmarkWidget->rect().width()/2));
    } else {
        // move bookmark items to the left, still visible
        to.moveLeft(geometry().right() - 60);
    }
    bookmarkAnim->setStartValue(from);
    bookmarkAnim->setEndValue(to);

    historyAnim->setEasingCurve(QEasingCurve::OutCubic);
    bookmarkAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_slideAnimationGroup->addAnimation(historyAnim);
    m_slideAnimationGroup->addAnimation(bookmarkAnim);
    m_slideAnimationGroup->start(QAbstractAnimation::KeepWhenStopped);
}

void HomeView::destroyViewItems()
{
    m_bookmarkWidget->removeAll();
    m_historyWidget->removeAll();
}

void HomeView::createViewItems()
{
    // recreate?
    destroyViewItems();
    //
    createBookmarkContent();
    createHistoryContent();
}

void HomeView::createBookmarkContent()
{
    UrlList* list = BookmarkStore::instance()->list();
    for (int i = 0; i < list->size(); ++i) {
        ListTileItem* newTileItem = new ListTileItem(m_bookmarkWidget, *list->at(i));
        m_bookmarkWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    m_bookmarkWidget->layoutTiles();
}

void HomeView::createHistoryContent()
{
    ThumbnailTileItem* newTileItem = 0;
    UrlList* list = HistoryStore::instance()->list();
    for (int i = 0; i < (s_maxHistoryTileNum - 1) && i < list->size(); ++i) {
        newTileItem = new ThumbnailTileItem(m_historyWidget, *list->at(i));
        m_historyWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    // FIXME: this 'All history'  is being leaked
    newTileItem = new ThumbnailTileItem(m_historyWidget, *(new UrlItem(QUrl(), "All history", 0)), false);
    m_historyWidget->addTile(*newTileItem);
    connectItem(*newTileItem);

    m_historyWidget->layoutTiles();
}

#include "HomeView.moc"
