#include "BrowsingView.h"

#include "WebView.h"
#include "WebPage.h"
#include "WebViewportItem.h"
#include "YberApplication.h"
#include "ApplicationWindow.h"
#include "Settings.h"
#include "BackingStoreVisualizerWidget.h"
#include "HomeView.h"
#include "TabSelectionView.h"
#include "Helpers.h"
#include "ProgressWidget.h"
#include "HistoryStore.h"
#include "BookmarkStore.h"
#include <QAction>
#include <QGraphicsLinearLayout>

#include "AutoScrollTest.h"

#if USE_DUI
#include <DuiTextEdit>
#include <DuiToolBar>
#include <DuiToolBarView>
#include <DuiAction>
#include <DuiButton>
#include <QToolBar>
#else
#include "AutoSelectLineEdit.h"
#include "WebViewport.h"

#include <QToolBar>
#include <QStyle>
#include <QMenuBar>


#endif

#include <QGraphicsProxyWidget>

#include "qwebframe.h"

/*!
   \class BrowsingView view displaying the web page, e.g the main browser UI.
*/

BrowsingView::BrowsingView(YberApplication&, QGraphicsItem *parent)
    : BrowsingViewBase(parent)
#if !USE_DUI
    , m_appWin(0)
    , m_centralWidget(new QGraphicsWidget(this))
#endif
    , m_activeWebView(0)
    , m_webInteractionProxy(new WebViewportItem())
    , m_tabSelectionView(0)
    , m_homeView(0)
    , m_urlEdit(0)
    , m_progressBox(0)
    , m_stopbackAction(0)
    , m_loadIndProgress(false)
    , m_autoScrollTest(0)
{
#if USE_DUI
    setPannableAreaInteractive(false);
    m_browsingViewport = new PannableViewport();
    m_browsingViewport->setWidget(m_webInteractionProxy);
#else
    m_browsingViewport = new WebViewport(m_webInteractionProxy);
#endif
    m_browsingViewport->setAutoRange(false);

    m_progressBox = new ProgressWidget(m_browsingViewport);
    // create and activate new window
    newWindow();

    YberWidget* w = centralWidget();
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, w);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0.);
    layout->addItem(m_browsingViewport);
    layout->addItem(createNavigationToolBar());
}

BrowsingView::~BrowsingView()
{
    delete m_autoScrollTest;
    deleteHomeView();
    deleteTabSelectionView();
}

void BrowsingView::connectSignals(WebView* currentView, WebView* oldView)
{
    if (oldView) {
        disconnect(oldView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        disconnect(oldView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
        disconnect(oldView, SIGNAL(urlChanged(const QUrl&)), this, SLOT(urlChanged(const QUrl&)));
        disconnect(oldView, SIGNAL(titleChanged(const QString&)), this, SLOT(setTitle(const QString&)));

        disconnect(oldView->page(), SIGNAL(loadStarted()), m_progressBox, SLOT(loadStarted()));
        disconnect(oldView->page(), SIGNAL(loadProgress(int)), m_progressBox, SLOT(progressChanged(int)));
        disconnect(oldView->page(), SIGNAL(loadFinished(bool)), m_progressBox, SLOT(loadFinished(bool)));
    #if !USE_DUI
        disconnect(oldView->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
    #endif
    }

    connect(currentView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(currentView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(currentView, SIGNAL(urlChanged(const QUrl&)), this, SLOT(urlChanged(const QUrl&)));
    connect(currentView, SIGNAL(titleChanged(const QString&)), this, SLOT(setTitle(const QString&)));

    connect(currentView->page(), SIGNAL(loadStarted()), m_progressBox, SLOT(loadStarted()));
    connect(currentView->page(), SIGNAL(loadProgress(int)), m_progressBox, SLOT(progressChanged(int)));
    connect(currentView->page(), SIGNAL(loadFinished(bool)), m_progressBox, SLOT(loadFinished(bool)));
#if !USE_DUI
    connect(currentView->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
#endif

}

YberWidget* BrowsingView::createNavigationToolBar()
{
#if USE_DUI
    DuiWidget* naviToolbar = new DuiWidget();
    naviToolbar->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    QGraphicsLinearLayout* toolbarLayout = new QGraphicsLinearLayout(Qt::Horizontal);

#define DEFINE_TOOLBAR_ITEM(text)  \
    {                              \
        DuiButton *btn = new DuiButton(text, naviToolbar);      \
        toolbarLayout->addItem(btn);                            \
    }
#define DEFINE_TOOLBAR_ITEM_CB(text, obj, slot)      \
    {                              \
        DuiButton *btn = new DuiButton(text, naviToolbar);      \
        connect(btn, SIGNAL(clicked(bool)), obj, slot);        \
        toolbarLayout->addItem(btn);                            \
    }

    DEFINE_TOOLBAR_ITEM("R");
    DEFINE_TOOLBAR_ITEM_CB("H", this, SLOT(toggleTabSelectionView()));

    m_urlEdit = new DuiTextEdit(DuiTextEditModel::SingleLine, QString(), naviToolbar);
    m_urlEdit->setViewType("toolbar");
    m_urlEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_urlEdit->setMinimumWidth(300);
    connect(m_urlEdit, SIGNAL(lostFocus(Qt::FocusReason)), SLOT(updateURL()));
    connect(m_urlEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

    toolbarLayout->addItem(m_urlEdit);

    toolbarLayout->addItem(naviToolbar);

    DEFINE_TOOLBAR_ITEM("B");
    DEFINE_TOOLBAR_ITEM_CB("F", this, SLOT(toggleFullScreen()));

#undef DEFINE_TOOLBAR_ITEM
    naviToolbar->setLayout(toolbarLayout);
#else

    QGraphicsProxyWidget* naviToolbar = new QGraphicsProxyWidget();
    m_toolbar = new QToolBar("Navigation");
    m_urlEdit = new UrlEditWidget(m_toolbar);
    m_urlEdit->setSizePolicy(QSizePolicy::Expanding, m_urlEdit->sizePolicy().verticalPolicy());
    m_urlEdit->setFocusPolicy(Qt::ClickFocus);
    connect(m_urlEdit, SIGNAL(textEdited(const QString&)), SLOT(urlTextEdited(const QString&)));
    connect(m_urlEdit, SIGNAL(editCancelled()), SLOT(updateURL()));
    connect(m_urlEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));
    connect(m_urlEdit, SIGNAL(focusChanged(bool)), SLOT(urlEditfocusChanged(bool)));
    m_toolbar->addAction(QIcon(":/data/icon/48x48/history_48.png"), "Tab selection", this, SLOT(toggleTabSelectionView()));
    m_toolbar->addAction(QIcon(":/data/icon/48x48/bookmarks_48.png"), "Add bookmark", this, SLOT(addBookmark()));
    m_toolbar->addWidget(m_urlEdit);
    m_stopbackAction = new QAction(QIcon(":/data/icon/48x48/back_48.png"), "Back", 0);
    connect(m_stopbackAction, SIGNAL(triggered()), this, SLOT(pageBack()));
    m_toolbar->addAction(m_stopbackAction);
    m_toolbar->addAction(QIcon(":/data/icon/48x48/screen_toggle_48.png"), "Fullscreen", this, SLOT(toggleFullScreen()));
    naviToolbar->setWidget(m_toolbar);
#endif
    return naviToolbar;
}

void BrowsingView::addBookmark()
{
    // FIXME: check if webviewport is active
    if (!m_activeWebView)
        return;
    if (m_activeWebView->url().isEmpty()) {
        notification("No page, no save.", m_browsingViewport);
        return;
    }
    // FIXME webkit returns empty favicon
    BookmarkStore::instance()->add(m_activeWebView->url(), m_activeWebView->title(), QIcon());
    notification("Bookmark saved.", m_browsingViewport);
}

void BrowsingView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    YberWidget* w = centralWidget();
    w->setGeometry(QRectF(w->pos(), size()));
    w->setPreferredSize(size());
    updateViews();
    m_progressBox->updateGeometry(m_browsingViewport->rect());

#if !USE_DUI
    if (!applicationWindow()->updatesEnabled()) {
        QSizeF dsz = m_browsingViewport->size() - m_sizeBeforeResize;
        QPointF d(dsz.width(), dsz.height());
        m_browsingViewport->setPanPos(m_browsingViewport->panPos() + d);
        applicationWindow()->setUpdatesEnabled(true);
    }
#endif

}

void BrowsingView::load(const QUrl& url)
{
    if (url.isValid())
        m_activeWebView->load(url.toString());
}

void BrowsingView::stopLoad()
{
    m_activeWebView->triggerPageAction(QWebPage::Stop);
}

void BrowsingView::pageBack()
{
    m_activeWebView->triggerPageAction(QWebPage::Back);
}

void BrowsingView::showHomeView()
{
    if (m_homeView && m_homeView->isActive())
        return;
    createHomeView();
#if USE_DUI
    m_homeView->appear(0);
#else
    m_homeView->appear(applicationWindow());
#endif
}

void BrowsingView::hideHomeView()
{
    if (m_homeView && m_homeView->isActive())
        m_homeView->disappear();
}

#if !USE_DUI

void BrowsingView::appear(ApplicationWindow *window)
{
    m_appWin = window;
    window->setPage(this);
    window->setMenuBar(createMenu(window));
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

void BrowsingView::setActiveWindow(WebView* webView)
{
    // FIXME
    if (webView == m_activeWebView)
        return;
    // invalidate url bar
    if (m_urlEdit)
        m_urlEdit->setText(webView->url().toString());
    connectSignals(webView, m_activeWebView);
    if (m_activeWebView)
        m_activeWebView->hide();
    webView->show();
    m_activeWebView = webView;
    m_webInteractionProxy->setWebView(webView);
    m_webInteractionProxy->setPos(QPointF(0,0));
}

void BrowsingView::destroyWindow(WebView* webView)
{
    // FIXME: create a new window on last window destory. better idea?
    if (m_windowList.size() == 1)
        newWindow();
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
    WebView* webView = new WebView();
    m_windowList.append(webView);
    webView->setPage(new WebPage(webView, this));
    setActiveWindow(webView);
    return webView;
}

void BrowsingView::createHomeView() 
{
    if (m_homeView)
        return;
    // FIXME: home view invokes should be moved to app framework
    m_homeView = new HomeView();
    m_homeView->setGeometry(m_browsingViewport->rect());
    connect(m_homeView, SIGNAL(disappeared()), this, SLOT(deleteHomeView()));
    connect(m_homeView, SIGNAL(pageSelected(const QUrl&)), this, SLOT(load(const QUrl&)));
}

void BrowsingView::deleteHomeView()
{
    delete m_homeView;
    m_homeView = 0;
}

void BrowsingView::createTabSelectionView() 
{
    if (m_tabSelectionView)
        return;
    // FIXME: home view invokes should be moved to app framework
    m_tabSelectionView = new TabSelectionView();
    m_tabSelectionView->setWindowList(m_windowList, *m_activeWebView);
    m_tabSelectionView->setGeometry(m_browsingViewport->rect());
    connect(m_tabSelectionView, SIGNAL(disappeared()), this, SLOT(deleteTabSelectionView()));
    connect(m_tabSelectionView, SIGNAL(windowSelected(WebView*)), this, SLOT(setActiveWindow(WebView*)));
    connect(m_tabSelectionView, SIGNAL(windowClosed(WebView*)), this, SLOT(destroyWindow(WebView*)));
    connect(m_tabSelectionView, SIGNAL(windowCreated()), this, SLOT(newWindow()));
}

void BrowsingView::deleteTabSelectionView()
{
    delete m_tabSelectionView;
    m_tabSelectionView = 0;
}

void BrowsingView::toggleTabSelectionView()
{
    createTabSelectionView();

    if (m_tabSelectionView->isActive())
        m_tabSelectionView->disappear();
    else
        m_tabSelectionView->appear(applicationWindow());
}

void BrowsingView::changeLocation()
{
    // nullify on hitting enter. end  of editing.
    m_lastEnteredText.resize(0);
    hideHomeView();
    if (!m_urlEdit)
        return;
    load(urlFromUserInput(m_urlEdit->text()));
}

void BrowsingView::urlTextEdited(const QString& newText)
{
    if (!Settings::instance()->autoCompleteEnabled())
        return;

    QString text = newText;
    if (m_urlEdit && m_urlEdit->selectionStart() > -1)
        text = newText.left(m_urlEdit->selectionStart());
    // autocomplete only when adding text, not when deleting or backspacing
    if (text.size() > m_lastEnteredText.size()) {
        // todo: make it async
        QString match = HistoryStore::instance()->match(text);
        if (match.size()) {
            m_urlEdit->setText(match);
            m_urlEdit->setCursorPosition(text.size());
            m_urlEdit->setSelection(text.size(), match.size() - text.size());
        }
    }
    m_lastEnteredText = text;
}

void BrowsingView::urlEditfocusChanged(bool focused)
{
    if (focused) {
        showHomeView();
    } else {
        // FIXME: this is a hack to not to get urleditor focused back
        // when the toolbar is focused, only when the actual edior is focused
        // currently, even if a button gets pressed, the urleditor gets 
        // focused back, if it was focused the last time
        m_toolbar->widgetForAction(m_stopbackAction)->setFocus(Qt::MouseFocusReason);
    }
}

void BrowsingView::updateHistoryStore(bool successLoad)
{
    // render thumbnail
    QImage* thumbnail = 0;
    UrlItem* item = HistoryStore::instance()->get(m_activeWebView->page()->mainFrame()->url().toString());
    // update thumbnail even if load failed (cancelled?) when there is no thumbnail yet.
    bool update = successLoad || !item || (item && !item->thumbnailAvailable());

    if (update) {
        QSize thumbnailSize(500, 400);
        thumbnail = new QImage(thumbnailSize.width(), thumbnailSize.height(), QImage::Format_RGB32);    
        QPainter p(thumbnail);
        m_activeWebView->page()->mainFrame()->render(&p, QWebFrame::ContentsLayer, QRegion(0, 0, thumbnailSize.width(), thumbnailSize.height()));
    }
    HistoryStore::instance()->accessed(m_activeWebView->page()->mainFrame()->url(), m_activeWebView->page()->mainFrame()->title(), thumbnail);
}

void BrowsingView::setLoadInProgress(bool loadInProgress)
{
    m_loadIndProgress = loadInProgress;
    toggleStopBackIcon();
}

void BrowsingView::loadStarted()
{
    setLoadInProgress(true);
}

void BrowsingView::loadFinished(bool success)
{
    setLoadInProgress(false);
    updateURL();
    updateHistoryStore(success);
//    QTimer::singleShot(1000, this, SLOT(updateHistoryStore()));
}

void BrowsingView::urlChanged(const QUrl& url)
{
    if (!m_urlEdit)
        return;
    m_urlEdit->setText(url.toString());
}

void BrowsingView::updateURL()
{
    urlChanged(m_activeWebView->url());
}

#if !USE_DUI
void BrowsingView::setTitle(const QString& title)
{
    QString t(title.trimmed());
    
    if (t.isEmpty())
        setTitle(QCoreApplication::instance()->applicationName());
    else
        if (applicationWindow())
            applicationWindow()->setTitle(t);
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

    if (applicationWindow()->isFullScreen())
        applicationWindow()->show();
    else
        applicationWindow()->showFullScreen();
#endif
}

void BrowsingView::updateViews()
{
    if (m_homeView)
        m_homeView->setGeometry(m_browsingViewport->geometry());

    if (m_tabSelectionView)
        m_tabSelectionView->setGeometry(m_browsingViewport->geometry());
}

void BrowsingView::prepareForResize()
{
#if !USE_DUI
    applicationWindow()->setUpdatesEnabled(false);
    m_sizeBeforeResize = m_browsingViewport->size();
#endif
}

void BrowsingView::startAutoScrollTest()
{
    delete m_autoScrollTest;
    m_autoScrollTest = new AutoScrollTest(this);
    connect(m_autoScrollTest, SIGNAL(finished()), this, SLOT(finishedAutoScrollTest()));
    m_autoScrollTest->starScrollTest();
}

void BrowsingView::finishedAutoScrollTest()
{
    delete m_autoScrollTest;
    m_autoScrollTest = 0;
}

void BrowsingView::toggleStopBackIcon()
{
    m_stopbackAction->setIcon(QIcon(m_loadIndProgress ? ":/data/icon/48x48/stop_48.png" : ":/data/icon/48x48/back_48.png"));
    m_stopbackAction->setIconText(m_loadIndProgress ? "Stop" : "Back");
    
    disconnect(m_stopbackAction, SIGNAL(triggered()), this, m_loadIndProgress ? SLOT(pageBack()) : SLOT(stopLoad()));
    connect(m_stopbackAction, SIGNAL(triggered()), this, m_loadIndProgress ? SLOT(stopLoad()) : SLOT(pageBack()));
}

