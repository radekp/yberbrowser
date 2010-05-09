#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QDebug>

#include "PopupView.h"
#include "UrlItem.h"
#include "HistoryStore.h"

class PopupWidget : public TileBaseWidget {
    Q_OBJECT
public:
    PopupWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags = 0) : TileBaseWidget("Search result", parent, wFlags) {}

    void layoutTiles();
};

void PopupWidget::layoutTiles()
{
    int vTileNum = m_tileList.size();
    int hTileNum = 1;
    int tileHeight = 65;

    QRectF r(rect());
    r.setHeight(tileHeight * m_tileList.size());
    
    // adjust it to the bottom url bar
    QRectF gRect(geometry());
    QRectF parentRect(parentWidget()->geometry());

    // too few items to cover the view?
    if (r.height() <= parentRect.height()) {
        gRect = parentRect;
        gRect.setTop(gRect.height() - r.height() - s_viewMargin);
        setGeometry(gRect);
    } else {
        setGeometry(parentRect);
    }

    // adjust the height of the view
    setMinimumHeight(r.height());
    setMaximumWidth(rect().width());

    // vertical adjustment
    r.adjust(10, 0, 0, -10);
    doLayoutTiles(r, hTileNum, r.width() - 60, vTileNum, tileHeight, 0, 0);
}

PopupView::PopupView(const QString& filterText, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(parent, wFlags)
    , m_popupWidget(new PopupWidget(this, wFlags))
    , m_pannableContainer(new PannableTileContainer(this, wFlags))
    , m_filterText(filterText)
{
    m_popupWidget->setZValue(1);
    m_pannableContainer->setWidget(m_popupWidget);
    connect(m_popupWidget, SIGNAL(closeWidget(void)), this, SLOT(disappear()));
}

PopupView::~PopupView()
{
    delete m_popupWidget;
    delete m_pannableContainer;
}

void PopupView::setGeometry(const QRectF& rect)
{
    TileSelectionViewBase::setGeometry(rect);

    m_pannableContainer->setGeometry(rect);
    m_popupWidget->setGeometry(rect);
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
    emit urlSelected(item->urlItem()->m_url);
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
