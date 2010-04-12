#include "BrowsingView.h"

#include "WebView.h"
#include "WebPage.h"
#include "WebViewportItem.h"
#include "YberApplication.h"
#include "ApplicationWindow.h"
#include "Settings.h"
#include "BackingStoreVisualizerWidget.h"
#include "HistoryView.h"
#include "Helpers.h"
#include "ProgressWidget.h"
#include "UrlStore.h"
#include <QAction>
#include <QGraphicsLinearLayout>

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
#include <QGLWidget>

/*!
   \class BrowsingView view displaying the web page, e.g the main browser UI.
*/

BrowsingView::BrowsingView(YberApplication&, QGraphicsItem *parent)
    : BrowsingViewBase(parent)
#if !USE_DUI
    , m_appWin(0)
    , m_centralWidget(new QGraphicsWidget(this))
#endif
    , m_backingStoreVisualizer(0)
    , m_historyView(0)
    , m_stopbackAction(0)
    , m_loadIndProgress(false)
{
#if USE_DUI
    setPannableAreaInteractive(false);
#endif
    m_webView = new WebView();
    m_page = new WebPage(m_webView);
    m_webView->setPage(m_page);
    WebViewportItem* webInteractionProxy = new WebViewportItem(m_webView);

#if USE_DUI
    m_browsingViewport = new PannableViewport();
    m_browsingViewport->setWidget(webInteractionProxy);
#else
    m_browsingViewport = new WebViewport(webInteractionProxy);
#endif
    m_browsingViewport->setAutoRange(false);

    YberWidget* w = centralWidget();
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, w);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0.);
    layout->addItem(m_browsingViewport);
    layout->addItem(createNavigationToolBar());

    m_progressBox = new ProgressWidget(m_browsingViewport);
    connectSignals();
}

void BrowsingView::connectSignals()
{
    connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(m_webView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(m_webView, SIGNAL(urlChanged(const QUrl&)), this, SLOT(urlChanged(const QUrl&)));
    connect(m_webView, SIGNAL(titleChanged(const QString&)), this, SLOT(setTitle(const QString&)));
    //connect(m_webPage, SIGNAL(windowCloseRequested()), this, SLOT(close()));

    connect(m_webView->page(), SIGNAL(loadStarted()), m_progressBox, SLOT(loadStarted()));
    connect(m_webView->page(), SIGNAL(loadProgress(int)), m_progressBox, SLOT(progressChanged(int)));
    connect(m_webView->page(), SIGNAL(loadFinished(bool)), m_progressBox, SLOT(loadFinished(bool)));
#if !USE_DUI
    connect(m_webView->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), m_browsingViewport, SLOT(reset()));
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
    DEFINE_TOOLBAR_ITEM_CB("H", this, SLOT(toggleHistoryView()));

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
    QToolBar* qtoolbar = new QToolBar("Navigation");
    m_urlEdit = new UrlEditWidget(qtoolbar);
    m_urlEdit->setSizePolicy(QSizePolicy::Expanding, m_urlEdit->sizePolicy().verticalPolicy());
    connect(m_urlEdit, SIGNAL(textEdited(const QString&)), SLOT(urlTextEdited(const QString&)));
    connect(m_urlEdit, SIGNAL(editCancelled()), SLOT(updateURL()));
    connect(m_urlEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));
    qtoolbar->addAction(QIcon("history_48.png"), "History", this, SLOT(toggleHistoryView()));
    qtoolbar->addWidget(m_urlEdit);
    m_stopbackAction = new QAction(QIcon("back_48.png"), "Back", 0);
    connect(m_stopbackAction, SIGNAL(triggered()), this, SLOT(pageBack()));
    qtoolbar->addAction(m_stopbackAction);
    qtoolbar->addAction(QIcon("screen_toggle_48.png"), "Fullscreen", this, SLOT(toggleFullScreen()));
    naviToolbar->setWidget(qtoolbar);
#endif
    return naviToolbar;
}

YberWidget* BrowsingView::navigationToolbar()
{
    return (YberWidget*)centralWidget()->layout()->itemAt(1);
}

void BrowsingView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    YberWidget* w = centralWidget();
    w->setGeometry(QRectF(w->pos(), size()));
    w->setPreferredSize(size());
    updateHistoryView();
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
        m_webView->load(url.toString());

/*    m_vp->updateGeometry();
    qDebug() << m_webView->size() << " " << m_vp->size();
    qDebug() << m_vp->sizeHint(Qt::PreferredSize) << m_vp->sizeHint(Qt::MinimumSize) << m_vp->sizeHint(Qt::MaximumSize) << m_webView->sizeHint(Qt::PreferredSize, QSize());
*/
}

void BrowsingView::stopLoad()
{
    m_webView->triggerPageAction(QWebPage::Stop);
}

void BrowsingView::pageBack()
{
    m_webView->triggerPageAction(QWebPage::Back);
}

void BrowsingView::showHistoryView()
{
    createHistoryView();
#if USE_DUI
    m_historyView->appear(0);
#else
    m_historyView->appear(applicationWindow());
#endif
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
    QWebPage* page = m_page;
    QMenuBar* menuBar = new QMenuBar(parent);

    QMenu* fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(new QAction("New Window", this));
    fileMenu->addAction(new QAction("Close", this));

    QMenu* viewMenu = menuBar->addMenu("&Navigation");
    viewMenu->addAction(page->action(QWebPage::Back));
    viewMenu->addAction(page->action(QWebPage::Forward));
    viewMenu->addAction(page->action(QWebPage::Stop));
    viewMenu->addAction(page->action(QWebPage::Reload));

    QMenu* developerMenu = menuBar->addMenu("&Developer");
    
    // fps
    QAction* fpsAction = new QAction("Show FPS", this);
    fpsAction->setCheckable(true);
    fpsAction->setChecked(Settings::instance()->FPSEnabled());
    connect(fpsAction, SIGNAL(toggled(bool)), this, SLOT(showFPSChanged(bool)));
    developerMenu->addAction(fpsAction);
    // tiling visualization
    QAction* tileAction = new QAction("Show tiles", this);
    tileAction->setCheckable(true);
    tileAction->setChecked(Settings::instance()->tileVisualizationEnabled());
    connect(tileAction, SIGNAL(toggled(bool)), this, SLOT(showTilesChanged(bool)));
    developerMenu->addAction(tileAction);
    return menuBar;
}
#endif

void BrowsingView::showFPSChanged(bool )
{
#if 0
    Settings::instance()->enableFPS(checked);
    setFPSCalculation(checked);
#endif
}

void BrowsingView::setFPSCalculation(bool )
{
#if 0
    if (fpsOn) {
        m_fpsTimerId = startTimer(s_fpsTimerInterval);
        m_fpsTicks = m_webViewItem->fpsTicks();
        m_fpsTimestamp.start();
    } else
        killTimer(m_fpsTimerId);
#endif
}

void BrowsingView::showTilesChanged(bool showTiles)
{
    Settings::instance()->enableTileVisualization(showTiles);

    if (showTiles) {
        if (!m_backingStoreVisualizer) {
            m_backingStoreVisualizer = new BackingStoreVisualizerWidget(m_webView, centralWidget());
        }
    } else {
        delete m_backingStoreVisualizer;
        m_backingStoreVisualizer = 0;
    }
}

BrowsingView* BrowsingView::newWindow(const QString &)
{
    return 0;
}

void BrowsingView::createHistoryView() 
{
    if (m_historyView)
        return;
    // FIXME: history view invokes should be moved to app framework
    m_historyView = new HistoryView();
    updateHistoryView();
    connect(m_historyView, SIGNAL(disappeared()), this, SLOT(deleteHistoryView()));
    connect(m_historyView, SIGNAL(urlSelected(const QUrl&)), this, SLOT(load(const QUrl&)));
}

void BrowsingView::deleteHistoryView()
{
    delete m_historyView;
    m_historyView = 0;
}

void BrowsingView::toggleHistoryView()
{
    createHistoryView();

    if (m_historyView->isActive())
        m_historyView->disappear();
    else
        showHistoryView();
}

void BrowsingView::hideHistoryView()
{
    if (m_historyView && m_historyView->isActive())
        m_historyView->disappear();
}

void BrowsingView::changeLocation()
{
    // nullify on hitting enter. end  of editing.
    m_lastEnteredText.resize(0);
    hideHistoryView();
    load(urlFromUserInput(m_urlEdit->text()));
}

void BrowsingView::urlTextEdited(const QString& newText)
{
    if (!Settings::instance()->autoCompleteEnabled())
        return;

    QString text = newText;
    if (m_urlEdit->selectionStart() > -1)
        text = newText.left(m_urlEdit->selectionStart());
    // autocomplete only when adding text, not when deleting or backspacing
    if (text.size() > m_lastEnteredText.size()) {
        // todo: make it async
        QString match = UrlStore::instance()->match(text);
        if (match.size()) {
            m_urlEdit->setText(match);
            m_urlEdit->setCursorPosition(text.size());
            m_urlEdit->setSelection(text.size(), match.size() - text.size());
        }
    }
    m_lastEnteredText = text;
}

void BrowsingView::updateUrlStore()
{
    // render thumbnail
    // todo: render thumbnail if it is really needed
    // todo: update them, right now thumbnails get saved only when accessed first
    QImage* thumbnail = 0;
    //if (!m_urlStore->contains(m_webViewItem->url().toString())) 
    {
        QSize thumbnailSize(500, 400);
        thumbnail = new QImage(thumbnailSize.width(), thumbnailSize.height(), QImage::Format_RGB32);    
        QPainter p(thumbnail);
        m_page->mainFrame()->render(&p, QWebFrame::ContentsLayer, QRegion(0, 0, thumbnailSize.width(), thumbnailSize.height()));
    }
    UrlStore::instance()->accessed(m_page->mainFrame()->url(), m_page->mainFrame()->title(), thumbnail);
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
    if (success)
        QTimer::singleShot(1000, this, SLOT(updateUrlStore()));
}

void BrowsingView::urlChanged(const QUrl& url)
{
    m_urlEdit->setText(url.toString());
}

void BrowsingView::updateURL()
{
    urlChanged(m_webView->url());
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

void BrowsingView::updateHistoryView()
{
    if (m_historyView)
        m_historyView->setGeometry(m_browsingViewport->geometry());
}

void BrowsingView::prepareForResize()
{
#if !USE_DUI
    applicationWindow()->setUpdatesEnabled(false);
    m_sizeBeforeResize = m_browsingViewport->size();
#endif
}

void BrowsingView::toggleStopBackIcon()
{
    m_stopbackAction->setIcon(QIcon(m_loadIndProgress ? "stop_48.png" : "back_48.png"));
    m_stopbackAction->setIconText(m_loadIndProgress ? "Stop" : "Back");
    
    disconnect(m_stopbackAction, SIGNAL(triggered()), this, m_loadIndProgress ? SLOT(pageBack()) : SLOT(stopLoad()));
    connect(m_stopbackAction, SIGNAL(triggered()), this, m_loadIndProgress ? SLOT(stopLoad()) : SLOT(pageBack()));
}

