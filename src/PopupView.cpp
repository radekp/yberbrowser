#include "PopupView.h"
#include <QTimer>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>

#include "UrlItem.h"
#include "HistoryStore.h"
#include "TileContainerWidget.h"
#include "PannableTileContainer.h"

class Suggest : public QObject {
    Q_OBJECT
public:
    Suggest();
    void start(const QString& text);
    void stop();
    QList<QString>* suggestions() { return &m_suggestions; }

Q_SIGNALS:
    void suggestionsAvailable();

private Q_SLOTS:
    void loadFinished(bool success);
    
private:
    bool m_loading;
    QGraphicsWebView m_view;
    QList<QString> m_suggestions;
};

Suggest::Suggest()
    : m_loading(false)
{
    connect(&m_view, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));    
}

void Suggest::start(const QString& text)
{
    if (m_loading)
        m_view.triggerPageAction(QWebPage::Stop);
    m_loading = true;
    m_view.load(QUrl("http://suggestqueries.google.com/complete/search?qu=" + text));
}

void Suggest::stop() 
{
    if (!m_loading)
        return;

    m_view.triggerPageAction(QWebPage::Stop);
}

void Suggest::loadFinished(bool success)
{
    m_loading = false;
    m_suggestions.clear();

    if (!success)
        return;
    QString json = m_view.page()->mainFrame()->toPlainText();
    int start = json.indexOf("(");
    if (start != -1) {
        json = json.mid(start);

        QScriptEngine engine;
        QScriptValue sc = engine.evaluate(json);

        if (sc.property(1).isArray()) {
            QScriptValueIterator it(sc.property(1));
            while (it.hasNext()) {
                it.next();
                if (it.value().isArray()) {
                    // only the first value is needed
                    QScriptValueIterator itt(it.value());
                    itt.next();
                    m_suggestions.append(itt.value().toString());
                }
            }
        }
    }
    emit suggestionsAvailable();
}

PopupView::PopupView(QGraphicsItem* parent, QPixmap* bckg, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(TileSelectionViewBase::UrlPopup, bckg, parent, wFlags)
    , m_suggest(new Suggest())
    , m_suggestTimer(new QTimer(this))
    , m_popupWidget(new PopupWidget(this, wFlags))
    , m_pannableContainer(new PannableTileContainer(this, wFlags))
{
    m_pannableContainer->setWidget(m_popupWidget);
    connect(m_popupWidget, SIGNAL(closeWidget(void)), this, SLOT(closeViewSoon()));
    connect(m_suggestTimer, SIGNAL(timeout()), this, SLOT(startSuggest()));
    connect(m_suggest, SIGNAL(suggestionsAvailable()), this, SLOT(populateSuggestion()));
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
    m_suggestTimer->start(500);
    m_suggest->stop();

    m_filterText = text;
    destroyViewItems();
    createViewItems();
}

void PopupView::startSuggest()
{
    m_suggestTimer->stop();
    m_suggest->start(m_filterText);
}

void PopupView::populateSuggestion()
{
    destroyViewItems();
    createViewItems();
}

void PopupView::tileItemActivated(TileItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);

    QUrl url = item->urlItem()->m_url;
    // FIXME this is ugly but ok as temp
    if (item->urlItem()->m_url.toString() == "google suggest")
        url = "http://www.google.com/search?q=" + item->urlItem()->m_title;      
    emit pageSelected(url);
}

void PopupView::destroyViewItems()
{
    m_popupWidget->removeAll();
}

void PopupView::createViewItems()
{
    UrlList matchedItems;
    HistoryStore::instance()->match(m_filterText, matchedItems);
    QList<QString>* suggestList = m_suggest->suggestions();

    // FIXME let the urlitem leak for now
    // add suggest items to the top
    for (int i = 0; i < suggestList->size() && i < (matchedItems.isEmpty() ? 5 : 2) ; ++i) {
        ListTileItem* suggestItem = new ListTileItem(m_popupWidget, *(new UrlItem(QUrl("google suggest"), suggestList->at(i), 0)));
        m_popupWidget->addTile(*suggestItem);
        connectItem(*suggestItem);
    }

    if (matchedItems.isEmpty()) {
        if (suggestList->isEmpty())
            m_popupWidget->addTile(*(new ListTileItem(m_popupWidget, *(new UrlItem(QUrl(), "no match", 0)))));
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
