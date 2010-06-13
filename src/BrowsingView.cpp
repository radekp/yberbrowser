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
#include "WebView.h"
#include "WebPage.h"
#include "WebViewportItem.h"
#include "YberApplication.h"
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

#if defined(Q_WS_MAEMO_5) && !defined(QT_NO_OPENGL)
#include <QGLWidget>
#endif

#if USE_DUI
#include <DuiTextEdit>
#include <DuiToolBar>
#include <DuiToolBarView>
#include <DuiAction>
#include <DuiButton>
#include <QToolBar>
#else
#include <QMenuBar>
#include "WebViewport.h"
#endif

const int s_maxWindows = 6;

/*!
   \class BrowsingView view displaying the web page, e.g the main browser UI.
*/
BrowsingView::BrowsingView(YberApplication&, QGraphicsItem *parent)
    : BrowsingViewBase(parent)
    , m_activeWebView(0)
    , m_webInteractionProxy(new WebViewportItem(this))
    , m_autoScrollTest(0)
    , m_homeView(0)
    , m_initialHomeWidget(HomeView::VisitedPages)
    , m_toolbarWidget(new ToolbarWidget(this))
    , m_appWin(0)
{
#if USE_DUI
    setPannableAreaInteractive(false);
    m_browsingViewport = new PannableViewport();
    m_browsingViewport->setWidget(m_webInteractionProxy);
#else
    m_browsingViewport = new WebViewport(m_webInteractionProxy, this);
#endif
    m_browsingViewport->setAutoRange(false);
#ifdef PERFECT_ATTACHED_TOOLBAR
    m_browsingViewport->setAttachedWidget(m_toolbarWidget);
#else
    m_browsingViewport->setOffsetWidget(m_toolbarWidget);
#endif

    connect(m_toolbarWidget, SIGNAL(bookmarkPressed()), SLOT(addBookmark()));
    connect(m_toolbarWidget, SIGNAL(backPressed()), SLOT(pageBack()));
    connect(m_toolbarWidget, SIGNAL(cancelPressed()), SLOT(stopLoad()));
    connect(m_toolbarWidget, SIGNAL(urlEditingFinished(const QString&)), SLOT(urlEditingFinished(const QString&)));
    connect(m_toolbarWidget, SIGNAL(urlEditorFocusChanged(bool)), SLOT(urlEditfocusChanged(bool)));
    // create and activate new window
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
        disconnect(oldView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
        disconnect(oldView, SIGNAL(urlChanged(const QUrl&)), this, SLOT(urlChanged(const QUrl&)));
        disconnect(oldView, SIGNAL(titleChanged(const QString&)), this, SLOT(setTitle(const QString&)));
    #if !USE_DUI
        disconnect(oldView->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
    #endif
    }

    connect(currentView, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    connect(currentView, SIGNAL(loadProgress(int)), SLOT(progressChanged(int)));
    connect(currentView, SIGNAL(loadStarted()), SLOT(loadStarted()));
    connect(currentView, SIGNAL(urlChanged(const QUrl&)), SLOT(urlChanged(const QUrl&)));
    connect(currentView, SIGNAL(titleChanged(const QString&)), SLOT(setTitle(const QString&)));
#if !USE_DUI
    connect(currentView->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
#endif
}

void BrowsingView::addBookmark()
{
    if (m_activeWebView->url().isEmpty()) {
        notification("No page, no save.", this);
        return;
    }
    // webkit returns empty favicon
    BookmarkStore::instance()->add(m_activeWebView->url(), m_activeWebView->title());
    notification("Bookmark saved.", this);
}

void BrowsingView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    m_webInteractionProxy->resize(size());
    m_browsingViewport->resize(size());

    if (m_homeView) {
        m_homeView->resize(QSize(3*size().width(), size().height()));
        m_homeView->updateBackground(webviewSnapshot());
    }

    QRectF r(rect());
    r.setHeight(ToolbarWidget::height());
    m_toolbarWidget->setRect(r);

#if !USE_DUI
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
    // default bckg
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
    m_activeWebView->triggerPageAction(QWebPage::Stop);
}

void BrowsingView::pageBack()
{
    m_activeWebView->triggerPageAction(QWebPage::Back);
}

#if !USE_DUI
void BrowsingView::appear(ApplicationWindow* window)
{
    m_appWin = window;
    window->setPage(this);
    window->setMenuBar(createMenu(window));
    window->scene()->addItem(this);
    // create home view on startup
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
    // create a new window on last window destory. better idea?
    if (m_windowList.size() == 1) {
        // on last window close, let's just close the home view, if it is up. revisit it later.
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
    // invalidate url bar
    m_toolbarWidget->setTextIfUnfocused(webView->url().isEmpty() ? "no page loaded yet" : webView->url().toString());
    connectWebViewSignals(webView, m_activeWebView);
    if (m_activeWebView)
        m_activeWebView->hide();
    webView->show();
    m_activeWebView = webView;
    m_webInteractionProxy->setWebView(webView);
    m_webInteractionProxy->setPos(QPointF(0,0));
    // view bckg needs to be updated
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
    WebView* webView = new WebView();
    m_windowList.append(webView);
    webView->setPage(new WebPage(webView, this));
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

    // create and display new home view
    m_homeView = new HomeView(type, webviewSnapshot(), this);
    m_homeView->setWindowList(m_windowList);
    connect(m_homeView, SIGNAL(pageSelected(const QUrl&)), this, SLOT(load(const QUrl&)));
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
    bool exist = HistoryStore::instance()->contains(m_activeWebView->page()->mainFrame()->url().toString());
    // update thumbnail even if load failed (cancelled?) when this is the first access.
    bool update = successLoad || !exist;

    if (update) {
        QSizeF thumbnailSize(size());
        thumbnail = new QImage(thumbnailSize.width(), thumbnailSize.height(), QImage::Format_RGB32);    
        QPainter p(thumbnail);
        m_activeWebView->page()->mainFrame()->render(&p, QWebFrame::ContentsLayer, QRegion(0, 0, thumbnailSize.width(), thumbnailSize.height()));
    }
    HistoryStore::instance()->accessed(m_activeWebView->url(), m_activeWebView->page()->mainFrame()->title(), thumbnail);
}

void BrowsingView::loadStarted()
{
    m_toolbarWidget->setProgress(1);
}

void BrowsingView::progressChanged(int progress)
{
    m_toolbarWidget->setProgress(progress);
}

void BrowsingView::loadFinished(bool success)
{
    m_toolbarWidget->setProgress(0);
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

#if !USE_DUI
void BrowsingView::setTitle(const QString& title)
{
    QString t(title.trimmed());
    
    if (t.isEmpty())
        setTitle(QCoreApplication::instance()->applicationName());
    else
        if (m_appWin)
            m_appWin->setTitle(t);
}
#endif

void BrowsingView::toggleFullScreen()
{
#if USE_DUI
    if (componentDisplayMode(DuiApplicationPage::AllComponents) == DuiApplicationPageModel::Show)
        setComponentsDisplayMode(DuiApplicationPage::AllComponents, DuiApplicationPageModel::Hide);
    else
        setComponentsDisplayMode(DuiApplicationPage::AllComponents, DuiApplicationPageModel::Show);
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
#if !USE_DUI
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

QPixmap* BrowsingView::webviewSnapshot()
{
    QSizeF thumbnailSize(size());
    QImage thumbnail(thumbnailSize.width(), thumbnailSize.height(), QImage::Format_RGB32);    

    QPainter p(&thumbnail);

    if (m_activeWebView && !m_activeWebView->url().isEmpty()) {
        qreal scale = m_activeWebView->scale();
        QStyleOptionGraphicsItem sItem;
        sItem.exposedRect = QRectF(QPointF(0, 0), thumbnailSize/scale);
        p.scale(scale, scale);
        //FIXME until the right api is figured out
//        m_activeWebView->paint(&p, &sItem);
        m_activeWebView->page()->mainFrame()->render(&p, QWebFrame::ContentsLayer, QRegion(0, 0, thumbnailSize.width()/scale, thumbnailSize.height()/scale));
        p.fillRect(sItem.exposedRect, QColor(0, 0, 0, 198));
    } else {
        p.fillRect(thumbnail.rect(), QColor(30, 30, 30));
    }
    return new QPixmap(QPixmap::fromImage(thumbnail));
}


