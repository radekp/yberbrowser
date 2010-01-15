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
#include <QLabel>
#include <QToolBar>

#if defined(Q_WS_MAEMO_5)
#include <QtDBus>
#include <QtMaemo5>
#include <mce/mode-names.h>
#include <mce/dbus-names.h>
#endif


#include "MainWindow.h"
#include "MainView.h"
#include "WebViewportItem.h"
#include "UrlStore.h"

static const float s_zoomScaleKeyStep = .2;
static const int s_fpsTimerInterval = 1000;

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
    , m_naviToolbar(0)
    , m_urlEdit(0)
    , m_fpsBox(0)
    , m_fpsTicks(0)
    , m_fpsTimerId(0)
{
    init();
    
    m_urlStore = 0;
    if (!m_settings.m_disableAutoComplete)
        m_urlStore = new UrlStore;


#if defined(Q_WS_MAEMO_5)
    QDBusConnection::systemBus().connect(QString(), MCE_SIGNAL_PATH, MCE_SIGNAL_IF,
                                         MCE_DEVICE_ORIENTATION_SIG,
                                         this,
                                         SLOT(orientationChanged(QString)));
#endif
}

MainWindow::~MainWindow()
{
    killTimer(m_fpsTimerId);
    delete m_urlStore;
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
    connect(m_webViewItem, SIGNAL(urlChanged(const QUrl&)), this, SLOT(urlChanged(const QUrl&)));
    connect(m_webViewItem, SIGNAL(titleChanged(const QString&)), this, SLOT(setWindowTitle(const QString&)));
    connect(m_webViewItem->page(), SIGNAL(windowCloseRequested()), this, SLOT(close()));

    buildUI();
    setLoadInProgress(false);
    setFPSCalculation(m_settings.m_showFPS);
}

void MainWindow::load(const QString& url)
{
    QUrl deducedUrl = MainWindow::urlFromUserInput(url);
    if (!deducedUrl.isValid())
        deducedUrl = QUrl("http://" + url + "/");

    m_urlEdit->setText(deducedUrl.toEncoded());
    m_webViewItem->load(deducedUrl);
    m_webViewItem->setFocus(Qt::OtherFocusReason);
}

void MainWindow::loadStarted()
{
    setLoadInProgress(true);
}

void MainWindow::setLoadInProgress(bool flag)
{
    m_stopAction->setVisible(flag);
    m_reloadAction->setVisible(!flag);
}

QWebPage* MainWindow::page() const
{
    return m_webViewItem->page();
}

MainView* MainWindow::view() {
    return m_view;
}

void MainWindow::changeLocation()
{
    // nullify on hitting enter. end of editing.
    m_lastEnteredText.resize(0);
    load(m_urlEdit->text());
}

void MainWindow::urlTextEdited(const QString& newText)
{
    if (!m_urlStore)
        return;

    QString text = newText;
    if (m_urlEdit->selectionStart() > -1)
        text = newText.left(m_urlEdit->selectionStart());
    // autocomplete only when adding text, not when deleting or backspacing
    if (text.size() > m_lastEnteredText.size()) {
        // todo: make it async
        QString match = m_urlStore->match(text);
        if (match.size()) {
            m_urlEdit->setText(match);
            m_urlEdit->setCursorPosition(text.size());
            m_urlEdit->setSelection(text.size(), match.size() - text.size());
        }
    }
    m_lastEnteredText = text;
}

void MainWindow::loadFinished(bool success)
{
    setLoadInProgress(false);
    urlChanged(m_webViewItem->url());
    if (success && m_urlStore)
        m_urlStore->accessed(m_webViewItem->url());
}

void MainWindow::urlChanged(const QUrl& url)
{
    m_urlEdit->setText(url.toString());
}

void MainWindow::showFPSChanged(bool checked) 
{
    m_settings.m_showFPS = checked;
    setFPSCalculation(m_settings.m_showFPS);
    buildToolbar();
}

void MainWindow::setFPSCalculation(bool fpsOn)
{
    if (fpsOn) {
        m_fpsTimerId = startTimer(s_fpsTimerInterval);
        m_fpsTicks = m_webViewItem->fpsTicks();
        m_fpsTimestamp.start();
    } else
        killTimer(m_fpsTimerId);
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
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Window", this, SLOT(newWindow()));
    fileMenu->addAction("Close", this, SLOT(close()));

    QMenu* viewMenu = menuBar()->addMenu("&Navigation");
    viewMenu->addAction(page->action(QWebPage::Back));
    viewMenu->addAction(page->action(QWebPage::Forward));
    viewMenu->addAction(page->action(QWebPage::Stop));
    viewMenu->addAction(page->action(QWebPage::Reload));

    QMenu* developerMenu = menuBar()->addMenu("&Developer");
    QAction* fpsAction = new QAction("Show FPS", developerMenu);
    fpsAction->setCheckable(true);
    fpsAction->setChecked(m_settings.m_showFPS);
    connect(fpsAction, SIGNAL(toggled(bool)), this, SLOT(showFPSChanged(bool)));
    developerMenu->addAction(fpsAction);
    
    buildToolbar();
}

void MainWindow::buildToolbar()
{
    QWebPage* page = m_webViewItem->page();
    if (!m_naviToolbar) {
        m_naviToolbar = new QToolBar("Navigation", this);
        addToolBar(m_naviToolbar);
    }
    // todo: find out if there is a way to remove widgets from toolbar, withouth trashing them all
    m_naviToolbar->clear();

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setSizePolicy(QSizePolicy::Expanding, m_urlEdit->sizePolicy().verticalPolicy());
    connect(m_urlEdit, SIGNAL(textEdited(const QString&)), SLOT(urlTextEdited(const QString&)));
    connect(m_urlEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

    QStyle *s = style();
    QAction* zoomInAction = new QAction(s->standardIcon(QStyle::SP_ArrowUp), "Zoom &in", m_naviToolbar);
    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    m_naviToolbar->addAction(zoomInAction);

    QAction* zoomOutAction = new QAction(s->standardIcon(QStyle::SP_ArrowDown), "Zoom &out", m_naviToolbar);
    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
    m_naviToolbar->addAction(zoomOutAction);

    m_naviToolbar->addAction(page->action(QWebPage::Back));
    m_naviToolbar->addAction(page->action(QWebPage::Forward));
    m_naviToolbar->addAction(m_reloadAction = page->action(QWebPage::Reload));
    m_naviToolbar->addAction(m_stopAction = page->action(QWebPage::Stop));
    m_naviToolbar->addWidget(m_urlEdit);

    if (m_settings.m_showFPS) {
        m_fpsBox = new QLabel("fps:0.0", this);
        m_fpsBox->setSizePolicy(QSizePolicy::Fixed, m_fpsBox->sizePolicy().verticalPolicy());
        m_naviToolbar->addSeparator();
        m_naviToolbar->addWidget(m_fpsBox);
    }

    if (m_settings.m_disableToolbar)
        m_naviToolbar->hide();
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
        if (event->modifiers() & Qt::ControlModifier) {
            if (event->key() == Qt::Key_I) {
                zoomIn();
                event->accept();
                return;
            } else if (event->key() == Qt::Key_O) {
                zoomOut();
                event->accept();
                return;
            }
#if defined(Q_WS_MAEMO_5)
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

void MainWindow::timerEvent(QTimerEvent *)
{
    if (!m_settings.m_showFPS)
        return;
    double dt = m_fpsTimestamp.restart();
    double dticks = m_webViewItem->fpsTicks() - m_fpsTicks;
    double d = 0;
    if (dt)
        d = (dticks *  1000.) / dt;
    m_fpsBox->setText(QString("FPS: %1").arg(d, 0, 'f', 1));

    m_fpsTicks = m_webViewItem->fpsTicks();
}

void MainWindow::zoomIn()
{
    WebViewportItem *viewportItem = m_view->interactionItem();
    qDebug() << __FUNCTION__ << viewportItem->zoomScale();
    

    viewportItem->animateZoomScaleTo(viewportItem->zoomScale() + s_zoomScaleKeyStep);
    qDebug() << viewportItem->zoomScale();
    
}

void MainWindow::zoomOut()
{
    WebViewportItem *viewportItem = m_view->interactionItem();
    viewportItem->animateZoomScaleTo(viewportItem->zoomScale() - s_zoomScaleKeyStep);
}

#if defined(Q_WS_MAEMO_5)
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
