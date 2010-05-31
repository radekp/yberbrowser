/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "PopupView.h"
#include "UrlItem.h"
#include "HistoryStore.h"
#include "TileContainerWidget.h"
#include "PannableViewport.h"

#include <QTimer>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>

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

PopupView::PopupView(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(TileSelectionViewBase::UrlPopup, 0, parent, wFlags)
    , m_suggest(new Suggest())
    , m_suggestTimer(new QTimer(this))
    , m_bckg(new QGraphicsRectItem(rect(), this))
    , m_popupWidget(new PopupWidget(this, wFlags))
    , m_pannableContainer(new PannableViewport(this, wFlags))
{
    m_pannableContainer->setWidget(m_popupWidget);
    connect(m_popupWidget, SIGNAL(closeWidget(void)), this, SLOT(closeViewSoon()));
    connect(m_suggestTimer, SIGNAL(timeout()), this, SLOT(startSuggest()));
    connect(m_suggest, SIGNAL(suggestionsAvailable()), this, SLOT(populateSuggestion()));
    m_bckg->setBrush(QColor(60, 60, 60, 220));
}

PopupView::~PopupView()
{
    delete m_pannableContainer;
}

void PopupView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    m_pannableContainer->setGeometry(rect());
    m_popupWidget->resize(rect().size());
    m_bckg->setRect(rect());
    TileSelectionViewBase::resizeEvent(event);
}

void PopupView::setFilterText(const QString& text)
{
#ifdef QTSCRIPT_FIX_AVAILABLE
    m_suggestTimer->start(500);
    m_suggest->stop();
#endif

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

    QUrl url = item->urlItem()->url();
    // FIXME this is ugly but ok as temp
    if (url.toString() == "google suggest")
        url = "http://www.google.com/search?q=" + item->urlItem()->title();      
    emit pageSelected(url);
}

void PopupView::tileItemClosed(TileItem* item)
{
    TileSelectionViewBase::tileItemClosed(item);
    m_popupWidget->removeTile(*item);
}

void PopupView::tileItemEditingMode(TileItem* item)
{
    TileSelectionViewBase::tileItemEditingMode(item);

    m_popupWidget->setEditMode(!m_popupWidget->editMode());
    update();
}

void PopupView::resetContainerSize()
{
    m_popupWidget->setMinimumHeight(0);
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
            ListTileItem* newTileItem = new ListTileItem(m_popupWidget, matchedItems.at(i));
            m_popupWidget->addTile(*newTileItem);
            connectItem(*newTileItem);
        }
    }
    m_popupWidget->layoutTiles();
}

#include "PopupView.moc"
