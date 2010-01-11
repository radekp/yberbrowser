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

#if defined(ENABLE_MAEMO5)
#include <QtDBus>
#include <QtMaemo5>
#include <mce/mode-names.h>
#include <mce/dbus-names.h>
#endif


#include "MainWindow.h"
#include "MainView.h"
#include "WebViewportItem.h"

static const float s_zoomScaleKeyStep = .2;

QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    MainWindow* mw = m_ownerWindow->createWindow();
    mw->show();
    return mw->page();
}


MainWindow::MainWindow(QNetworkProxy* proxy, Settings settings)
    : QMainWindow()
    , m_view(new MainView(this, settings))
    , m_scene(new QGraphicsScene(this))
    , m_webViewItem(new WebView)
    , m_page(0)
    , m_proxy(proxy)
    , m_settings(settings)
{
    init();

#if defined(ENABLE_MAEMO5)
    QDBusConnection::systemBus().connect(QString(), MCE_SIGNAL_PATH, MCE_SIGNAL_IF,
                                         MCE_DEVICE_ORIENTATION_SIG,
                                         this,
                                         SLOT(orientationChanged(QString)));
#endif
}

MainWindow::~MainWindow()
{
}

MainWindow* MainWindow::createWindow()
{
    return new MainWindow(m_proxy, m_settings);
}

void MainWindow::init()
{
    resize(800, 480);

    setAttribute(Qt::WA_DeleteOnClose);
    m_page = new WebPage(m_webViewItem, this);
    if (m_proxy)
        m_page->networkAccessManager()->setProxy(*m_proxy);

    m_webViewItem->setPage(m_page);
    
    m_webViewItem->setResizesToContent(true);
#if defined(WEBKIT_SUPPORTS_TILE_CACHE)
    m_page->mainFrame()->setTileCacheEnabled(!m_settings.m_disableTiling);
#endif

    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    m_view->setScene(m_scene);

    setCentralWidget(m_view);
    m_view->setWebView(m_webViewItem);

    connect(m_webViewItem, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(m_webViewItem, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(m_webViewItem, SIGNAL(titleChanged(const QString&)), this, SLOT(setWindowTitle(const QString&)));
    connect(m_webViewItem->page(), SIGNAL(windowCloseRequested()), this, SLOT(close()));

    buildUI();
    setLoadInProgress(false);
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

void MainWindow::loadStarted()
{
    setLoadInProgress(true);
}

void MainWindow::setLoadInProgress(bool /*flag*/)
{
/*  m_stopAction->setVisible(flag);
    m_reloadAction->setVisible(!flag);*/
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
    setLoadInProgress(false);

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
    bar->addAction(m_reloadAction = page->action(QWebPage::Reload));
    bar->addAction(m_stopAction = page->action(QWebPage::Stop));
    bar->addWidget(urlEdit);
    if (m_settings.m_disableToolbar)
        bar->hide();

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Window", this, SLOT(newWindow()));
    fileMenu->addAction("Close", this, SLOT(close()));

    QMenu* viewMenu = menuBar()->addMenu("&Navigation");
    viewMenu->addAction(page->action(QWebPage::Back));
    viewMenu->addAction(page->action(QWebPage::Forward));
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

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (m_view->interactionItem()) {
        WebViewportItem *viewportItem = m_view->interactionItem();

        if (event->modifiers() & Qt::ControlModifier) {
            if (event->key() == Qt::Key_I) {
                viewportItem->animateZoomScaleTo(viewportItem->zoomScale() + s_zoomScaleKeyStep);
                event->accept();
                return;
            } else if (event->key() == Qt::Key_O) {
                viewportItem->animateZoomScaleTo(viewportItem->zoomScale() - s_zoomScaleKeyStep);
                event->accept();
                return;
            }
#if defined(ENABLE_MAEMO5)
            else if (event->key() == Qt::Key_L) {
                setLandscape();
            } else if (event->key() == Qt::Key_P) {
                setPortrait();
            }
#endif
        }
    }

    QMainWindow::keyPressEvent(event);
}

#if defined(ENABLE_MAEMO5)
bool MainWindow::event(QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::WindowActivate:
        QDBusConnection::systemBus().call(
            QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH,
                                           MCE_REQUEST_IF,
                                           MCE_ACCELEROMETER_ENABLE_REQ));
        break;
    case QEvent::WindowDeactivate:
        QDBusConnection::systemBus().call(
            QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH,
                                           MCE_REQUEST_IF,
                                           MCE_ACCELEROMETER_DISABLE_REQ));
        break;
    default:
        break;
    }

    return QWidget::event(ev);
}

void MainWindow::orientationChanged(const QString &newOrientation)
{
    if (newOrientation == QLatin1String(MCE_ORIENTATION_PORTRAIT))
        setPortrait();
    else
        setLandscape();
}

void MainWindow::setLandscape()
{
        setAttribute(Qt::WA_Maemo5ForceLandscapeOrientation, true);
}

void MainWindow::setPortrait()
{
        setAttribute(Qt::WA_Maemo5ForcePortraitOrientation, true);
}

#endif
