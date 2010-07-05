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

#include "BrowsingView.h"

#if USE_WEBKIT2
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPageNamespace.h>
#else
#include "WebPage.h"
#endif

#include "WebViewportItem.h"
#include "WebView.h"
#include "ApplicationWindow.h"
#include "Settings.h"
#include "HomeView.h"
#include "PopupView.h"
#include "Helpers.h"
#include "ProgressWidget.h"
#include "HistoryStore.h"
#include "BookmarkStore.h"
#include "AutoScrollTest.h"
#include "ToolbarWidget.h"
#include "qwebframe.h"

#include <QAction>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QStyleOptionGraphicsItem>
#include <QPainter>

#if USE_MEEGOTOUCH
#include <MTextEdit>
#include <MToolBar>
#include <MToolBarView>
#include <MAction>
#include <MButton>
#include <QToolBar>
#else
#include <QMenuBar>
#endif
#include "WebViewport.h"

namespace {

const int s_maxWindows = 6;

}


/*!
   \class BrowsingView view displaying the web page, e.g the main browser UI.
*/
BrowsingView::BrowsingView(QGraphicsItem *parent)
    : BrowsingViewBase(parent)
    , m_activeWebView(0)
    , m_webInteractionProxy(new WebViewportItem(this))
    , m_autoScrollTest(0)
    , m_homeView(0)
    , m_initialHomeWidget(HomeView::VisitedPages)
    , m_toolbarWidget(new ToolbarWidget(this))
    , m_appWin(0)
{
    m_browsingViewport = new WebViewport(m_webInteractionProxy, this);
    m_browsingViewport->setAutoRange(false);
#ifdef PERFECT_ATTACHED_TOOLBAR
    m_browsingViewport->setAttachedWidget(m_toolbarWidget);
#else
    m_browsingViewport->setOffsetWidget(m_toolbarWidget);
#endif

    connect(m_toolbarWidget, SIGNAL(bookmarkPressed()), SLOT(addBookmark()));
    connect(m_toolbarWidget, SIGNAL(backPressed()), SLOT(pageBack()));
    connect(m_toolbarWidget, SIGNAL(cancelPressed()), SLOT(stopLoad()));
    connect(m_toolbarWidget, SIGNAL(urlEditingFinished(QString)), SLOT(urlEditingFinished(QString)));
    connect(m_toolbarWidget, SIGNAL(urlEditorFocusChanged(bool)), SLOT(urlEditfocusChanged(bool)));

#if USE_WEBKIT2
    m_context.adopt(WKContextGetSharedProcessContext());
#endif

    // Create and activate new window.
    newWindow();
}

BrowsingView::~BrowsingView()
{
    delete m_autoScrollTest;
    delete m_homeView;
    delete m_toolbarWidget;
}

void BrowsingView::connectWebViewSignals(WebView* currentView, WebView* oldView)
{
    if (oldView) {
        disconnect(oldView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        disconnect(oldView, SIGNAL(loadProgress(int)), this, SLOT(progressChanged(int)));
        disconnect(oldView, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
        disconnect(oldView, SIGNAL(titleChanged(QString)), this, SLOT(setTitle(QString)));
#if !USE_MEEGOTOUCH
#if USE_WEBKIT2
        disconnect(oldView->page(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
#else
        disconnect(oldView->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
#endif
#endif
    }

    connect(currentView, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    connect(currentView, SIGNAL(loadProgress(int)), SLOT(progressChanged(int)));
    connect(currentView, SIGNAL(urlChanged(QUrl)), SLOT(urlChanged(QUrl)));
    connect(currentView, SIGNAL(titleChanged(QString)), SLOT(setTitle(QString)));
#if !USE_MEEGOTOUCH
#if USE_WEBKIT2
    connect(currentView->page(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
#else
    connect(currentView->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
#endif
#endif
}

void BrowsingView::addBookmark()
{
    if (m_activeWebView->url().isEmpty()) {
        notification("No page, no save.", this);
        return;
    }
    // WebKit returns empty favicon, known bug.
    BookmarkStore::instance()->add(m_activeWebView->url(), m_activeWebView->title());
    notification("Bookmark saved.", this);
}

void BrowsingView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    m_browsingViewport->resize(size());
#if USE_WEBKIT2
    m_webInteractionProxy->setZoomScale(size().width() / m_webInteractionProxy->contentsSize().width());
#endif
    if (m_homeView) {
        m_homeView->resize(QSize(3 * size().width(), size().height()));
        m_homeView->updateBackground(webviewSnapshot());
    }

    QRectF r(rect());
    r.setHeight(ToolbarWidget::height());
    m_toolbarWidget->setRect(r);
#if !USE_MEEGOTOUCH
    if (!m_appWin->updatesEnabled()) {
        QSizeF dsz = m_browsingViewport->size() - m_sizeBeforeResize;
        QPointF d(dsz.width(), dsz.height());
        m_browsingViewport->setPanPos(m_browsingViewport->panPos() + d);
        m_appWin->setUpdatesEnabled(true);
    }
#endif
}

void BrowsingView::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    // Default background.
    painter->fillRect(rect(), QBrush(Qt::black));
}

void BrowsingView::load(const QUrl& url)
{
    deleteHomeView();
    if (url.isValid())
        m_activeWebView->load(url.toString());
    m_toolbarWidget->setTextIfUnfocused(url.toString());
}

void BrowsingView::stopLoad()
{
    m_activeWebView->stop();
}

void BrowsingView::pageBack()
{
    m_activeWebView->back();
}

#if !USE_MEEGOTOUCH
void BrowsingView::appear(ApplicationWindow* window)
{
    m_appWin = window;
    window->setPage(this);
    window->setMenuBar(createMenu(window));
    window->scene()->addItem(this);

    // Create home view on startup.
    createHomeView(HomeView::VisitedPages);
}

QMenuBar* BrowsingView::createMenu(QWidget* parent)
{
    QMenuBar* menuBar = new QMenuBar(parent);

    QMenu* fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(new QAction("Close", this));

    QMenu* developerMenu = menuBar->addMenu("&Developer");

    QAction* fpsTestAction = new QAction("FPS test", this);
    developerMenu->addAction(fpsTestAction);
    connect(fpsTestAction, SIGNAL(triggered(bool)), this, SLOT(startAutoScrollTest()));

    return menuBar;
}
#endif

void BrowsingView::windowSelected(WebView* webView)
{
    setActiveWindow(webView);
    deleteHomeView();
}

void BrowsingView::windowClosed(WebView* webView)
{
    // Create a new window on last window destory. better idea?
    if (m_windowList.size() == 1) {
        // On last window close, let's just close the home view, if it is up. revisit it later.
        deleteHomeView();
        newWindow();
    }
    destroyWindow(webView);
}

void BrowsingView::windowCreated()
{
    newWindow();
    if (m_homeView) {
        m_homeView->setActiveWidget(HomeView::VisitedPages);
        m_homeView->updateContent();
    } else {
        createHomeView(HomeView::VisitedPages);
    }
}

void BrowsingView::setActiveWindow(WebView* webView)
{
    if (webView == m_activeWebView)
        return;
    // Invalidate address bar.
    m_toolbarWidget->setTextIfUnfocused(webView->url().isEmpty() ? "No page loaded yet." : webView->url().toString());
    connectWebViewSignals(webView, m_activeWebView);
    if (m_activeWebView)
        m_activeWebView->hide();
    webView->show();
    m_activeWebView = webView;
    m_webInteractionProxy->setWebView(webView);
    m_webInteractionProxy->setPos(QPointF(0, 0));

    // View background needs to be updated.
    if (m_homeView)
        m_homeView->updateBackground(webviewSnapshot());
}

void BrowsingView::destroyWindow(WebView* webView)
{
    for (int i = 0; i < m_windowList.size(); ++i) {
        if (m_windowList.at(i) == webView) {
            // current? activate the next one, unless this is the last window
            if (webView == m_activeWebView)
                setActiveWindow(m_windowList.at((i == m_windowList.size() - 1) ? m_windowList.size() - 2 : i + 1));
            delete m_windowList.takeAt(i);
            break;
        }
    }
}

WebView* BrowsingView::newWindow()
{
    if (m_windowList.size() >= s_maxWindows)
        return 0;

#if USE_WEBKIT2
    WKRetainPtr<WKPageNamespaceRef> pageNamespace(AdoptWK, WKPageNamespaceCreate(m_context.get()));
    WebView* webView = new WebView(pageNamespace.get());
#else
    WebView* webView = new WebView();
    webView->setPage(new WebPage(webView, this));
#endif
    m_windowList.append(webView);
    setActiveWindow(webView);
    return webView;
}

void BrowsingView::createHomeView(HomeView::HomeWidgetType type)
{
    if (m_homeView) {
        if (m_homeView->activeWidget() != type)
            m_homeView->setActiveWidget(type);
        return;
    }

    // Create and display new home view.
    m_homeView = new HomeView(type, webviewSnapshot(), this);
    m_homeView->setWindowList(m_windowList);
    connect(m_homeView, SIGNAL(pageSelected(QUrl)), this, SLOT(load(QUrl)));
    connect(m_homeView, SIGNAL(windowSelected(WebView*)), this, SLOT(windowSelected(WebView*)));
    connect(m_homeView, SIGNAL(windowClosed(WebView*)), this, SLOT(windowClosed(WebView*)));
    connect(m_homeView, SIGNAL(windowCreated()), this, SLOT(windowCreated()));
    connect(m_homeView, SIGNAL(viewDismissed()), this, SLOT(deleteHomeView()));
    m_homeView->resize(QSize(3*size().width(), size().height()));
    m_homeView->appear();
    m_webInteractionProxy->hide();
}

void BrowsingView::deleteHomeView()
{
    if (!m_homeView)
        return;

    m_homeView->disappear();
    m_webInteractionProxy->show();
    // save homeview state, so that it is positioned back to the same view when reopened
    m_initialHomeWidget = m_homeView->activeWidget();
    // unless this is WindowSelect with empty page, users probably want to load something
    // and not going back to WindowSelect again
    if (m_initialHomeWidget == HomeView::WindowSelect && m_activeWebView->url().isEmpty())
        m_initialHomeWidget = HomeView::VisitedPages;

    delete m_homeView;
    m_homeView = 0;
}

void BrowsingView::urlEditingFinished(const QString& url)
{
    load(urlFromUserInput(url));
}

void BrowsingView::urlEditfocusChanged(bool focused)
{
    if (focused) {
        // bring keypad on when the homeview is already on and the user taps on the editor
        if (m_homeView)
            m_toolbarWidget->showKeypad();
        createHomeView(m_homeView ? m_homeView->activeWidget() : m_initialHomeWidget);
    }
}

void BrowsingView::updateHistoryStore(bool successLoad)
{
    // render thumbnail
    QImage* thumbnail = 0;
    bool exist = HistoryStore::instance()->contains(m_activeWebView->url().toString());
    // update thumbnail even if load failed (cancelled?) when this is the first access.
    bool update = successLoad || !exist;

    if (update) {
        QGraphicsPixmapItem* pixmapItem = webviewSnapshot(false);
        if (pixmapItem)
            thumbnail = new QImage(pixmapItem->pixmap().toImage());
        delete pixmapItem;
    }
    HistoryStore::instance()->accessed(m_activeWebView->url(), m_activeWebView->title(), thumbnail);
}

void BrowsingView::progressChanged(int progress)
{
    m_toolbarWidget->setProgress(progress);
}

void BrowsingView::loadFinished(bool success)
{
    if (success) {
        // I might be typing a new url while it is loading, when I press return I don't want the
        // address bar to changed the url to the one I was loading.
        urlChanged(m_activeWebView->url());
    }
    updateHistoryStore(success);
}

void BrowsingView::urlChanged(const QUrl& url)
{
    m_toolbarWidget->setTextIfUnfocused(url.toString());
}

#if !USE_MEEGOTOUCH
void BrowsingView::setTitle(const QString& title)
{
    QString t(title.trimmed());

    if (t.isEmpty())
        setTitle(QCoreApplication::instance()->applicationName());
    else {
        if (m_appWin)
            m_appWin->setTitle(t);
    }
}
#endif

void BrowsingView::toggleFullScreen()
{
#if USE_MEEGOTOUCH
    if (componentDisplayMode(MApplicationPage::AllComponents) == MApplicationPageModel::Show)
        setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
    else
        setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Show);
#else
    prepareForResize();

    if (m_appWin->isFullScreen())
        m_appWin->show();
    else
        m_appWin->showFullScreen();
#endif
}

void BrowsingView::prepareForResize()
{
#if !USE_MEEGOTOUCH
    m_appWin->setUpdatesEnabled(false);
    m_sizeBeforeResize = m_browsingViewport->size();
#endif
}

void BrowsingView::startAutoScrollTest()
{
    if (Settings::instance()->isFullScreen() && !m_appWin->isFullScreen())
        toggleFullScreen();
    delete m_autoScrollTest;
    m_autoScrollTest = new AutoScrollTest(m_browsingViewport, m_activeWebView, this);
    m_autoScrollTest->resize(rect().size());
    connect(m_autoScrollTest, SIGNAL(finished()), this, SLOT(finishedAutoScrollTest()));
    m_autoScrollTest->starScrollTest();
}

void BrowsingView::finishedAutoScrollTest()
{
    delete m_autoScrollTest;
    m_autoScrollTest = 0;
}

QGraphicsPixmapItem* BrowsingView::webviewSnapshot(bool darken)
{
    QSizeF thumbnailSize(size());
    QImage thumbnail(thumbnailSize.width(), thumbnailSize.height(), QImage::Format_RGB32);

    QPainter p(&thumbnail);

    if (m_activeWebView && !m_activeWebView->url().isEmpty()) {
        qreal scale = m_activeWebView->scale();
        QStyleOptionGraphicsItem sItem;
        sItem.exposedRect = QRectF(QPointF(0, 0), thumbnailSize/scale);
        p.scale(scale, scale);
        // FIXME until the right api is figured out.
        m_activeWebView->paint(&p, &sItem);
        //m_activeWebView->page()->mainFrame()->render(&p, QWebFrame::ContentsLayer, QRegion(0, 0, thumbnailSize.width()/scale, thumbnailSize.height()/scale));
        if (darken)
            p.fillRect(sItem.exposedRect, QColor(0, 0, 0, 198));
    } else
        p.fillRect(thumbnail.rect(), QColor(30, 30, 30));

    return new QGraphicsPixmapItem(QPixmap::fromImage(thumbnail));
}

