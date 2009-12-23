/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2009 Kenneth Christiansen <kenneth@webkit.org>
 * Copyright (C) 2009 Antonio Gomes <antonio.gomes@openbossa.org>
 * Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QDebug>
#include <QFile>
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


#include "AutoScroller.h"

QNetworkProxy* g_globalProxy;

bool g_disableToolbar;
bool g_useGL;

#if WEBKIT_SUPPORTS_TILE_CACHE
bool g_disableTiling;
#else
bool g_disableTiling = false;
#endif

static QUrl urlFromUserInput(const QString& string)
{
    QString input(string);
    QFileInfo fi(input);
    if (fi.exists() && fi.isRelative())
        input = fi.absoluteFilePath();

    return QUrl::fromUserInput(input);
}

class WebView : public QGraphicsWebView {
    Q_OBJECT

public:
    WebView(QGraphicsItem* parent = 0)
        : QGraphicsWebView(parent)
    {
    }
};

class WebPage : public QWebPage {
    Q_OBJECT

public:
    WebPage(QObject* parent = 0)
        : QWebPage(parent)
    {
        applyProxy();
    }
    virtual QWebPage* createWindow(QWebPage::WebWindowType);

private:
    void applyProxy()
    {
        if (g_globalProxy)
            networkAccessManager()->setProxy(*g_globalProxy);
    }
};

class MainView : public QGraphicsView {
    Q_OBJECT

public:
    MainView(QWidget* parent)
        : QGraphicsView(parent)
        , m_mainWidget(0)
    {
        if (g_useGL)
            setViewport(new QGLWidget);

        setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setInteractive(false);
        setOptimizationFlags(QGraphicsView::DontSavePainterState);

        if (!g_disableTiling) {
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        } else {
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }

        setFrameShape(QFrame::NoFrame);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setMainWidget(QGraphicsWidget* widget)
    {
        QRectF rect(QRect(QPoint(0, 0), size()));
        widget->setGeometry(rect);
        m_mainWidget = widget;
    }

    QGraphicsWidget* mainWidget()
    {
        return m_mainWidget;
    }

    void resizeEvent(QResizeEvent* event)
    {
        QGraphicsView::resizeEvent(event);
        if (!m_mainWidget)
            return;

        if (g_disableTiling) {
            QRectF rect(QPoint(0, 0), event->size());
            m_mainWidget->setGeometry(rect);
        }
    }

private:
    QGraphicsWidget* m_mainWidget;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow()
        : QMainWindow()
        , m_view(new MainView(this))
        , m_scene(new QGraphicsScene)
        , m_webViewItem(new WebView)
        , m_page(0)
    {
        init();
    }

    ~MainWindow()
    {
        delete m_page;
        delete m_webViewItem;
        delete m_scene;
    }

    void init()
    {
        setAttribute(Qt::WA_DeleteOnClose);

        m_webViewItem->setPage((m_page = new WebPage));
        m_webViewItem->setResizesToContent(true);
#if WEBKIT_SUPPORTS_TILE_CACHE
        m_page->mainFrame()->setTileCacheEnabled(!g_disableTiling);
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

    void load(const QString& url)
    {
        QUrl deducedUrl = urlFromUserInput(url);
        if (!deducedUrl.isValid())
            deducedUrl = QUrl("http://" + url + "/");

        urlEdit->setText(deducedUrl.toEncoded());
        m_webViewItem->load(deducedUrl);
        m_webViewItem->setFocus(Qt::OtherFocusReason);
    }

    QWebPage* page() const
    {
        return m_webViewItem->page();
    }

    void disableHildonDesktopCompositing() {
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
    MainView* view() { return m_view; }

protected slots:
    void changeLocation()
    {
        load(urlEdit->text());
    }

    void loadFinished(bool)
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

public slots:
    MainWindow* newWindow(const QString &url = QString())
    {
        MainWindow* mw = new MainWindow();
        mw->load(url);
        return mw;
    }

private:
    void buildUI()
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
        if (g_disableToolbar)
            bar->hide();

        QMenu* fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction("New Window", this, SLOT(newWindow()));
        fileMenu->addAction("Close", this, SLOT(close()));

        QMenu* viewMenu = menuBar()->addMenu("&View");
        viewMenu->addAction(page->action(QWebPage::Stop));
        viewMenu->addAction(page->action(QWebPage::Reload));
    }

private:
    MainView* m_view;
    QGraphicsScene* m_scene;
    WebView* m_webViewItem;
    WebPage* m_page;

    QLineEdit* urlEdit;
};


QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    MainWindow* mw = new MainWindow;
    mw->show();
    return mw->page();
}

void usage(const char* name);

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QUrl proxyUrl = urlFromUserInput(qgetenv("http_proxy"));
    if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
        int proxyPort = (proxyUrl.port() > 0) ? proxyUrl.port() : 8080;
        g_globalProxy = new QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyPort);
    }

    QString url = QString("file://%1/%2").arg(QDir::homePath()).arg(QLatin1String("index.html"));

    app.setApplicationName("yberbrowser");

    QWebSettings::setObjectCacheCapacities((16 * 1024 * 1024) / 8, (16 * 1024 * 1024) / 8, 16 * 1024 * 1024);
    QWebSettings::setMaximumPagesInCache(4);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);

    QStringList args = app.arguments();

    bool noFullscreen = false;
    bool autoScroll  = false;
    bool gotFlag = true;
    while (gotFlag) {
        if (args.count() > 1) {
            if (args.at(1) == "-w") {
                noFullscreen = true;
                args.removeAt(1);
            } else if (args.at(1) == "-t") {
                g_disableToolbar = true;
                args.removeAt(1);
            } else if (args.at(1) == "-g") {
                g_useGL = true;
                args.removeAt(1);
            } else if (args.at(1) == "-s") {
                autoScroll = true;
                args.removeAt(1);
            } else if (args.at(1) == "-c") {
                g_disableTiling = true;
                args.removeAt(1);
            } else if (args.at(1) == "-?" || args.at(1) == "-h" || args.at(1) == "--help") {
                usage(argv[0]);
                return EXIT_SUCCESS;
            } else {
                gotFlag = false;
            }
        } else {
            gotFlag = false;
        }
    }

    if (args.count() > 1)
        url = args.at(1);

    MainWindow* window = new MainWindow;
    window->load(url);
    if (noFullscreen)
        window->show();
    else {
        window->showFullScreen();
        window->disableHildonDesktopCompositing();
    }

    for (int i = 2; i < args.count(); i++) {
        window->newWindow(args.at(i));
        if (noFullscreen)
            window->show();
        else {
            window->showFullScreen();
            window->disableHildonDesktopCompositing();
        }
    }
    if (autoScroll) {
        AutoScroller* as = new AutoScroller;
        as->install(window->view(), window->view()->mainWidget());
    }

    int retval = app.exec();


    delete g_globalProxy;
    return retval;
}


void usage(const char* name) 
{
    QTextStream s(stderr);
    s << "usage: " << name << " [options] url" << endl;
    s << " -w disable fullscreen" << endl;
    s << " -t disable toolbar" << endl;
    s << " -g use glwidget as qgv viewport" << endl;
    s << " -s scroll" << endl;
    s << " -c disable tile cache" << endl;
    s << " -h|-?|--help help" << endl;
    s << endl;
    s << " use http_proxy env var to set http proxy" << endl;
}

#include "main.moc"
