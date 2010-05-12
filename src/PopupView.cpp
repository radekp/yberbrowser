#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QDebug>

#include "PopupView.h"
#include "UrlItem.h"
#include "HistoryStore.h"

const int s_searchItemTileHeight = 60;
const int s_tileMargin = 20;

class PopupWidget : public TileBaseWidget {
    Q_OBJECT
public:
    PopupWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) : TileBaseWidget("Search result", parent, wFlags) {}
    void layoutTiles();
};

void PopupWidget::layoutTiles()
{
    QRectF r(rect());
    r.setHeight(parentWidget()->size().height() - s_viewMargin);
    int popupHeight = doLayoutTiles(r, 1, r.height()/s_searchItemTileHeight, s_tileMargin, -1).height();
    // position to the bottom of the container, when there are too few items
    if (popupHeight < r.height()) 
        setGeometry(QRectF(QPointF(0, parentWidget()->size().height() - popupHeight - s_viewMargin), QSizeF(rect().width(), popupHeight)));
    else
        setGeometry(QRect(0, 0, r.width(), popupHeight));
}

PopupView::PopupView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(TileSelectionViewBase::UrlPopup, parent, wFlags)
    , m_popupWidget(new PopupWidget(this, wFlags))
    , m_pannableContainer(new PannableTileContainer(this, wFlags))
{
    m_pannableContainer->setWidget(m_popupWidget);
    connect(m_popupWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

PopupView::~PopupView()
{
    delete m_pannableContainer;
}

void PopupView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    m_pannableContainer->setGeometry(rect());
    m_popupWidget->resize(rect().size());

    TileSelectionViewBase::resizeEvent(event);
}

void PopupView::setFilterText(const QString& text)
{
    m_filterText = text;
    destroyViewItems();
    createViewItems();
}

void PopupView::tileItemActivated(TileItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);
    emit pageSelected(item->urlItem()->m_url);
}

void PopupView::destroyViewItems()
{
    m_popupWidget->removeAll();
}

void PopupView::createViewItems()
{
    UrlList matchedItems;
    HistoryStore::instance()->match(m_filterText, matchedItems);

    if (matchedItems.isEmpty()) {
        // FIXME let the urlitem leak for now
        ListTileItem* startTyping = new ListTileItem(m_popupWidget, *(new UrlItem(QUrl(), "no match", 0)));
        m_popupWidget->addTile(*startTyping);
    } else {
        for (int i = 0; i < matchedItems.size(); ++i) {
            ListTileItem* newTileItem = new ListTileItem(m_popupWidget, *matchedItems.at(i));
            m_popupWidget->addTile(*newTileItem);
            connectItem(*newTileItem);
        }
    }
    m_popupWidget->layoutTiles();
}

#include "PopupView.moc"
