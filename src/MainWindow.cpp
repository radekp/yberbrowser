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

#include <X11/Xlib.h>
#include <QX11Info>
#include <X11/Xatom.h>
#endif

#include "MainWindow.h"
#include "MainView.h"
#include "WebViewportItem.h"
#include "UrlStore.h"
#include "Settings.h"

static const float s_zoomScaleKeyStep = 1.4;
static const int s_fpsTimerInterval = 250;

// time limit for the zoom in/out button in milliseconds.
// This allows zoom in/out buttons to be held down and still
// the view will be interactive
static const int s_zoomInOutInterval = 100;

// timeout between clicking url bar and marking the url selected
static const int s_urlTapSelectAllTimeout = 200;


QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    MainWindow* mw = m_ownerWindow->createWindow();
    mw->show();
    return mw->page();
}

AutoSelectLineEdit::AutoSelectLineEdit(QWidget* parent)
        : QLineEdit(parent)
        , m_selectURLTimer(this)
{
    m_selectURLTimer.setSingleShot(true);
    m_selectURLTimer.setInterval(s_urlTapSelectAllTimeout);
    connect(&m_selectURLTimer, SIGNAL(timeout()), this, SLOT(selectAll()));
}

void AutoSelectLineEdit::focusInEvent(QFocusEvent*e)
{
    QLineEdit::focusInEvent(e);
    m_selectURLTimer.start();
}

void AutoSelectLineEdit::focusOutEvent(QFocusEvent*e)
{
    QLineEdit::focusOutEvent(e);
    m_selectURLTimer.stop();
    emit editCancelled();
    deselect();
}

MainWindow::MainWindow(QNetworkProxy* proxy, const QString& url)
    : QMainWindow()
    , m_mainView(new MainView(this))
    , m_scene(new QGraphicsScene(this))
    , m_webViewItem(new WebView)
    , m_page(0)
    , m_proxy(proxy)
    , m_naviToolbar(0)
    , m_urlEdit(0)
    , m_fpsBox(0)
    , m_fpsTicks(0)
    , m_fpsTimerId(0)
{
    init(!url.size());
    m_zoomInOutTimestamp.start();

#if defined(Q_WS_MAEMO_5)
    QDBusConnection::systemBus().connect(QString(), MCE_SIGNAL_PATH, MCE_SIGNAL_IF,
                                         MCE_DEVICE_ORIENTATION_SIG,
                                         this,
                                         SLOT(orientationChanged(QString)));
    grabIncreaseDecreaseKeys(this, true);
#endif
    if (url.size())
        load(url);
}

MainWindow::~MainWindow()
{
    killTimer(m_fpsTimerId);
}

MainWindow* MainWindow::createWindow()
{
    return new MainWindow(m_proxy, QString());
}

void MainWindow::init(bool historyOn)
{
    resize(800, 480);

    setAttribute(Qt::WA_DeleteOnClose);

    m_page = new WebPage(m_webViewItem, this);
    if (m_proxy)
        m_page->networkAccessManager()->setProxy(*m_proxy);

    m_webViewItem->setPage(m_page);
    
    m_webViewItem->setResizesToContent(true);
#if defined(WEBKIT_SUPPORTS_TILE_CACHE) && WEBKIT_SUPPORTS_TILE_CACHE
    m_page->mainFrame()->setTileCacheEnabled(Settings::instance()->tileCacheEnabled());
#endif

    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    m_mainView->setScene(m_scene);
    setCentralWidget(m_mainView);

    m_mainView->init(historyOn);
    m_mainView->setWebView(m_webViewItem);

    connect(m_webViewItem, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(m_webViewItem, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(m_webViewItem, SIGNAL(urlChanged(const QUrl&)), this, SLOT(urlChanged(const QUrl&)));
    connect(m_webViewItem, SIGNAL(titleChanged(const QString&)), this, SLOT(setWindowTitle(const QString&)));
    connect(m_webViewItem->page(), SIGNAL(windowCloseRequested()), this, SLOT(close()));

    buildUI();
    setLoadInProgress(false);
    setFPSCalculation(Settings::instance()->FPSEnabled());
    m_mainView->interactionItem()->showTiles(Settings::instance()->tileVisualizationEnabled());
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

MainView* MainWindow::mainView() {
    return m_mainView;
}

void MainWindow::changeLocation()
{
    // nullify on hitting enter. end  of editing.
    m_lastEnteredText.resize(0);
    load(m_urlEdit->text());
}

void MainWindow::urlTextEdited(const QString& newText)
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

void MainWindow::loadFinished(bool success)
{
    setLoadInProgress(false);
    updateURL();
    if (success)
        QTimer::singleShot(1000, this, SLOT(updateUrlStore()));
}

void MainWindow::updateUrlStore()
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
    UrlStore::instance()->accessed(m_webViewItem->url(), m_webViewItem->title(), thumbnail);
}

void MainWindow::urlChanged(const QUrl& url)
{
    m_urlEdit->setText(url.toString());
}

void MainWindow::updateURL()
{
    urlChanged(m_webViewItem->url());
}

void MainWindow::showFPSChanged(bool checked) 
{
    Settings::instance()->enableFPS(checked);
    setFPSCalculation(checked);
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

void MainWindow::showTilesChanged(bool checked)
{
    Settings::instance()->enableTileVisualization(checked);
    m_mainView->interactionItem()->showTiles(checked);
}

MainWindow* MainWindow::newWindow(const QString &url)
{
    return new MainWindow(0 , url);
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
    // fps
    QAction* fpsAction = new QAction("Show FPS", developerMenu);
    fpsAction->setCheckable(true);
    fpsAction->setChecked(Settings::instance()->FPSEnabled());
    connect(fpsAction, SIGNAL(toggled(bool)), this, SLOT(showFPSChanged(bool)));
    developerMenu->addAction(fpsAction);
    // tiling visualization
    QAction* tileAction = new QAction("Show tiles", developerMenu);
    tileAction->setCheckable(true);
    tileAction->setChecked(Settings::instance()->tileVisualizationEnabled());
    connect(tileAction, SIGNAL(toggled(bool)), this, SLOT(showTilesChanged(bool)));
    developerMenu->addAction(tileAction);

    buildToolbar();
}

void MainWindow::buildToolbar()
{
    QWebPage* page = m_webViewItem->page();
    if (!m_naviToolbar) {
        m_naviToolbar = new QToolBar("Navigation", this);
        addToolBar(m_naviToolbar);
    }
    // todo: find out if there is a way to remove widgets from toolbar, without trashing them all
    m_naviToolbar->clear();

    m_urlEdit = new AutoSelectLineEdit(this);
    m_urlEdit->setSizePolicy(QSizePolicy::Expanding, m_urlEdit->sizePolicy().verticalPolicy());
    connect(m_urlEdit, SIGNAL(textEdited(const QString&)), SLOT(urlTextEdited(const QString&)));
    connect(m_urlEdit, SIGNAL(editCancelled()), SLOT(updateURL()));
    connect(m_urlEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

    if (Settings::instance()->FPSEnabled()) {
        m_fpsBox = new QLabel("FPS:00.00", this);
        m_fpsBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_fpsBox->setFixedSize(m_fpsBox->width(), m_fpsBox->height());
        m_naviToolbar->addSeparator();
        m_naviToolbar->addWidget(m_fpsBox);
    }

    m_naviToolbar->addAction(m_reloadAction = page->action(QWebPage::Reload));
    m_naviToolbar->addAction(m_stopAction = page->action(QWebPage::Stop));
    m_historyAction = m_naviToolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon), "History", m_mainView, SLOT(toggleHistory()));
    m_naviToolbar->addWidget(m_urlEdit);
    m_naviToolbar->addAction(page->action(QWebPage::Back));
    if (!Settings::instance()->toolbarEnabled())
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
    if (m_mainView->interactionItem()) {
        if (event->modifiers() & Qt::ControlModifier) {
            switch (event->key()) {
            case Qt::Key_I:
                zoomIn();
                event->accept();
                return;
            case Qt::Key_O:
                zoomOut();
                event->accept();
                return;
#if defined(Q_WS_MAEMO_5)
            case Qt::Key_L:
                setLandscape();
                event->accept();
                return;
            case Qt::Key_P:
                setPortrait();
                event->accept();
                return;
#endif
            }
        }
#if defined(Q_WS_MAEMO_5)
        else {
            switch (event->key()) {
            case Qt::Key_F7:
                zoomIn();
                event->accept();
                return;
            case Qt::Key_F8:
                zoomOut();
                event->accept();
                return;
            }
        }
#endif
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::timerEvent(QTimerEvent *)
{
    if (!Settings::instance()->FPSEnabled())
        return;
    double dt = m_fpsTimestamp.restart();
    double dticks = m_webViewItem->fpsTicks() - m_fpsTicks;
    double d = 0;
    if (dt)
        d = (dticks *  1000.) / dt;
    m_fpsBox->setText(QString("FPS: %1").arg(d, 0, 'f', 2));

    m_fpsTicks = m_webViewItem->fpsTicks();
}

void MainWindow::zoomInOrOut(bool zoomIn)
{
    if (m_zoomInOutTimestamp.elapsed() < s_zoomInOutInterval)
        return;
    m_zoomInOutTimestamp.restart();

    WebViewportItem *viewportItem = m_mainView->interactionItem();
    qreal curScale = viewportItem->zoomScale();
    qreal newScale;

    if (zoomIn)
        newScale = curScale * s_zoomScaleKeyStep;
    else
        newScale = curScale / s_zoomScaleKeyStep;

    newScale = qBound(static_cast<qreal>(0.5), newScale, static_cast<qreal>(10));

    QSizeF viewportSize = viewportItem->size();

    QPointF vc(viewportSize.width()/2, viewportSize.height()/2);
    QPointF dc = m_webViewItem->mapFromScene(vc) * (newScale);
    QPointF p = viewportItem->clipPointToViewport(vc - dc, newScale);

    viewportItem->startZoomAnimTo(p, newScale);
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

void MainWindow::grabIncreaseDecreaseKeys(QWidget* window, bool grab)
{
    // Tell maemo-status-volume to grab/ungrab increase/decrease keys
    unsigned long val = (grab==true)?1:0;
    Atom atom;
    atom = XInternAtom( QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", 0);
    XChangeProperty (QX11Info::display(),
                     window->winId(),
                     atom,
                     XA_INTEGER,
                     32,
                     PropModeReplace,
                     (unsigned char *) &val,
                     1);
}

#endif
