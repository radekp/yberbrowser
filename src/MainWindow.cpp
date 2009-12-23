#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QNetworkRequest>
#include <QTextStream>
#include <QVector>
#include <QtGui>
#include <QtNetwork/QNetworkProxy>
#include <cstdio>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>
#include <QGLWidget>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>


#include "MainWindow.h"
#include "MainView.h"



QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    MainWindow* mw = m_ownerWindow->createWindow();
    mw->show();
    return mw->page();
}


MainWindow::MainWindow(QNetworkProxy* proxy, Settings settings)
    : QMainWindow()
    , m_view(new MainView(this, settings))
    , m_scene(new QGraphicsScene)
    , m_webViewItem(new WebView)
    , m_page(0)
    , m_proxy(proxy)
    , m_settings(settings)
{
    init();
}

MainWindow::~MainWindow()
{
    delete m_page;
    delete m_webViewItem;
    delete m_scene;
}

WebPage* MainWindow::createWebPage()
{
    WebPage* page = new WebPage(0, this);
    if (m_proxy)
        page->networkAccessManager()->setProxy(*m_proxy);
    return page;
}

MainWindow* MainWindow::createWindow()
{
    return new MainWindow(m_proxy, m_settings);
}
void MainWindow::init()
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_page = createWebPage();
    m_webViewItem->setPage(m_page);
    
    m_webViewItem->setResizesToContent(true);
#if WEBKIT_SUPPORTS_TILE_CACHE
    m_page->mainFrame()->setTileCacheEnabled(!m_settings.m_disableTiling);
#endif
    m_scene->addItem(m_webViewItem);
    m_scene->setActiveWindow(m_webViewItem);
    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    
    m_view->setScene(m_scene);

    setCentralWidget(m_view);

    m_view->setMainWidget(m_webViewItem);

    connect(m_webViewItem, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(m_webViewItem, SIGNAL(titleChanged(const QString&)), this, SLOT(setWindowTitle(const QString&)));
    connect(m_webViewItem->page(), SIGNAL(windowCloseRequested()), this, SLOT(close()));

    resize(640, 480);
    buildUI();

}

void MainWindow::load(const QString& url)
{
    QUrl deducedUrl = MainWindow::urlFromUserInput(url);
    if (!deducedUrl.isValid())
        deducedUrl = QUrl("http://" + url + "/");

    urlEdit->setText(deducedUrl.toEncoded());
    m_webViewItem->load(deducedUrl);
    m_webViewItem->setFocus(Qt::OtherFocusReason);
}

QWebPage* MainWindow::page() const
{
    return m_webViewItem->page();
}

void MainWindow::disableHildonDesktopCompositing() {
    // this should cause desktop compositing to be turned off for
    // fullscreen apps. This should speed up drawing. Unclear if
    // it does.
    // see also: hildon-desktop/src/mb/hd-comp-mgr.c
    unsigned long val = 1;
    Atom atom;
    atom = XInternAtom(QX11Info::display(), "_HILDON_NON_COMPOSITED_WINDOW", 0);
    XChangeProperty (QX11Info::display(),
                     winId(),
                     atom,
                     XA_INTEGER,
                     32,
                     PropModeReplace,
                     (unsigned char *) &val,
                     1);
}
MainView* MainWindow::view() {
    return m_view;
}

void MainWindow::changeLocation()
{
    load(urlEdit->text());
}

void MainWindow::loadFinished(bool)
{
    QUrl url = m_webViewItem->url();
    urlEdit->setText(url.toString());

    QUrl::FormattingOptions opts;
    opts |= QUrl::RemoveScheme;
    opts |= QUrl::RemoveUserInfo;
    opts |= QUrl::StripTrailingSlash;
    QString s = url.toString(opts);
    s = s.mid(2);
    if (s.isEmpty())
        return;
}

MainWindow* MainWindow::newWindow(const QString &url)
{
    MainWindow* mw = new MainWindow();
    mw->load(url);
    return mw;
}


void MainWindow::buildUI()
{
    QWebPage* page = m_webViewItem->page();
    urlEdit = new QLineEdit(this);
    urlEdit->setSizePolicy(QSizePolicy::Expanding, urlEdit->sizePolicy().verticalPolicy());
    connect(urlEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

    QToolBar* bar = addToolBar("Navigation");
    bar->addAction(page->action(QWebPage::Back));
    bar->addAction(page->action(QWebPage::Forward));
    bar->addAction(page->action(QWebPage::Reload));
    bar->addAction(page->action(QWebPage::Stop));
    bar->addWidget(urlEdit);
    if (m_settings.m_disableToolbar)
        bar->hide();

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Window", this, SLOT(newWindow()));
    fileMenu->addAction("Close", this, SLOT(close()));

    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(page->action(QWebPage::Stop));
    viewMenu->addAction(page->action(QWebPage::Reload));
}

QUrl MainWindow::urlFromUserInput(const QString& string)
{
    QString input(string);
    QFileInfo fi(input);
    if (fi.exists() && fi.isRelative())
        input = fi.absoluteFilePath();

    return QUrl::fromUserInput(input);
}

