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
#include <qwebhistory.h>

#include <QGLWidget>
#include <QtGlobal>

#include "MainView.h"
#include "MainWindow.h"
#include "WebViewportItem.h"

static const unsigned s_tileSize = 35;
static const unsigned s_progressBckgColor = 0xA0C6F3;
static const unsigned s_progressTextColor = 0x185488;
static QString s_initialProgressText("Loading...");

#define TILE_KEY(x,y) (x << 16 | y)

class TileItem : public QObject {
    Q_OBJECT
public:
    TileItem(unsigned hPos, unsigned vPos, bool visible, QGraphicsView* parent);
    ~TileItem();

    bool isActive() const;
    void setActive(bool active);
    void setVisible(bool visible);
    void painted();

private Q_SLOTS:
    void paintBlinkEnd();

private:    
    unsigned           m_vPos;
    unsigned           m_hPos;
    bool               m_active;
    unsigned           m_beingPainted;
    QGraphicsRectItem* m_rectItem;
    QGraphicsView*     m_parent;
};

TileItem::TileItem(unsigned hPos, unsigned vPos, bool visible, QGraphicsView* parent) 
    : m_vPos(vPos)
    , m_hPos(hPos)
    , m_active(false)
    , m_beingPainted(0)
    , m_rectItem(parent->scene()->addRect(hPos*s_tileSize, vPos*s_tileSize, s_tileSize, s_tileSize))
    , m_parent(parent)
{
    setVisible(visible);
    setActive(true);
}

TileItem::~TileItem()
{
    m_parent->scene()->removeItem(m_rectItem);
    delete m_rectItem;
}

bool TileItem::isActive() const 
{ 
    return m_active; 
}

void TileItem::setActive(bool active) 
{
    Q_ASSERT(!(active && m_active));
    // todo: it seems that tiles get stuck when navigating
    // from page A to B. filter duplicates out just for the sake of proper
    // visulazition, until the duplication is fixed
    if (active && m_active)
        qDebug() << "duplicate tile at:" << m_hPos << " " <<  m_vPos;
    
    m_active = active;
    m_rectItem->setBrush(QBrush(active?Qt::cyan:Qt::gray));
    m_rectItem->setOpacity(0.4);
}

void TileItem::setVisible(bool visible) 
{
    m_rectItem->setVisible(visible);
}

void TileItem::painted() 
{
    if (!m_rectItem->isVisible() || !m_active || m_beingPainted)
        return;
    m_beingPainted = 1;
    m_rectItem->setBrush(QBrush(Qt::red));
    QTimer::singleShot(1000, this, SLOT(paintBlinkEnd()));
}

void TileItem::paintBlinkEnd() 
{
    // dont keep getting reinvalidated 
    if (m_beingPainted == 1) {
        m_rectItem->setBrush(QBrush(m_active?Qt::cyan:Qt::gray));
        QTimer::singleShot(1000, this, SLOT(paintBlinkEnd()));
        m_beingPainted = 2;
        return;
    }
    m_beingPainted = 0;
}

// TODO:
// detect when user interaction has been done and do
// m_state = Interaction;

// to debug load events etc:
// make DEFINES+=-DENABLE_LOADEVENT_DEBUG
// this is needed in order to make load/back/forward etc
// as fast as possible, doing as little zooming changes as possible
// qt api is not very clear on the signal order. once the aspects
// of load signal order have been cleared, remove these ifdefs

MainView::MainView(MainWindow* window)
    : QGraphicsView(window)
    , m_mainWindow(window)
    , m_interactionItem(0)
    , m_state(InitialLoad)
    , m_webView(0)
    , m_tilesOn(false)
    , m_progressBox(0)
{
    if (window->settings().m_useGL)  {
	QGLFormat format = QGLFormat::defaultFormat();
        format.setSampleBuffers(false);

        QGLWidget *glWidget = new QGLWidget(format);    
        glWidget->setAutoFillBackground(false);

	setViewport(glWidget);
    }

    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

MainView::~MainView()
{
    // todo: figure out ownership
    delete m_progressBox;
}

void MainView::setWebView(WebView* webViewItem)
{
    if (webViewItem) {
        if (!m_interactionItem) {
            Q_ASSERT(scene());
            m_interactionItem = new WebViewportItem();
            scene()->addItem(m_interactionItem);
            scene()->setActiveWindow(m_interactionItem);
        }
        m_interactionItem->setWebView(m_webView = webViewItem);
        installSignalHandlers();
        updateSize();
    } else {
        m_interactionItem->setParent(0);
        delete m_interactionItem;
        m_interactionItem = 0;
    }
}

void MainView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateSize();
}

void MainView::updateSize()
{
    setUpdatesEnabled(false);
    if (!m_interactionItem)
        return;

    QRectF rect(QPointF(0, 0), size());
    scene()->setSceneRect(rect);

    m_interactionItem->setGeometry(rect);
    updateZoomScaleToPageWidth();
    setUpdatesEnabled(true);
    update();
}

WebView* MainView::webView()
{
    return m_webView;
}

void MainView::installSignalHandlers()
{
    connect(webView()->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), this, SLOT(resetState()));
    connect(webView()->page()->mainFrame(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(contentsSizeChanged(const QSize&)));
    connect(webView()->page(),SIGNAL(saveFrameStateRequested(QWebFrame*,QWebHistoryItem*)), SLOT(saveFrameState(QWebFrame*,QWebHistoryItem*)));
    connect(webView()->page(),SIGNAL(restoreFrameStateRequested(QWebFrame*)), SLOT(restoreFrameState(QWebFrame*)));
    connect(webView(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(webView(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(webView(), SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
    connect(webView()->page(),SIGNAL(tileCreated(unsigned, unsigned)), SLOT(tileCreated(unsigned, unsigned)));
    connect(webView()->page(),SIGNAL(tileRemoved(unsigned, unsigned)), SLOT(tileRemoved(unsigned, unsigned)));
    connect(webView()->page(),SIGNAL(tilePainted(unsigned, unsigned)), SLOT(tilePainted(unsigned, unsigned)));
    connect(webView()->page(),SIGNAL(tileCacheViewportScaleChanged()), SLOT(tileCacheViewportScaleChanged()));
}

void MainView::resetState()
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    m_state = InitialLoad;
    if (m_interactionItem) {
        // zoom will be updated in contentsSizeChanged
        m_interactionItem->resetState(false);
    }

    setUpdatesEnabled(true);
    update();
}

void MainView::loadStarted()
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    // progress indicator
    connect(scene(), SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(sceneRectChanged(const QRectF&)));
    // probably need to be changed this to something else, but not qprogress
    if (!m_progressBox) {
        m_progressBox = new QLabel();
        m_progressBox->setFrameStyle(QFrame::Panel);
        m_progressBox->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        const QPalette& p = m_progressBox->palette();
        QPalette m(p);
        m.setBrush(QPalette::Window, QBrush(QColor(s_progressBckgColor)));
        m.setColor(QPalette::WindowText, QColor(s_progressTextColor));
        m_progressBox->setPalette(m);
        scene()->addWidget(m_progressBox);
    }
    m_progressBox->setText(s_initialProgressText);
    m_progressBox->setGeometry(progressRect());
    m_progressBox->show();

    //setUpdatesEnabled(false);
}

void MainView::loadProgress(int progress)
{
    // todo: find out this magic 10% thing
    if (progress <= 10)
        return;
    m_progressBox->setText(QString::number(progress) + "%");
}

void MainView::loadFinished(bool)
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    m_progressBox->hide();
    disconnect(scene(), SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(sceneRectChanged(const QRectF&)));

    if (m_state == InitialLoad)
        m_state = Interaction;
}

void MainView::sceneRectChanged(const QRectF& /*rect*/)
{
    m_progressBox->setGeometry(progressRect());
}

void MainView::contentsSizeChanged(const QSize&)
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    // FIXME: QSize& arg vs contentsSize(): these  report different sizes
    // probably scrollbar thing
    if (m_state == InitialLoad) {
        updateZoomScaleToPageWidth();
    }
}

void MainView::updateZoomScaleToPageWidth()
{
    if (!m_interactionItem)
        return;

    QSize contentsSize = webView()->page()->mainFrame()->contentsSize();
    qreal targetScale = 1.;
    if (contentsSize.width()) {
        targetScale = static_cast<qreal>(m_interactionItem->size().width()) / contentsSize.width();
    }
    m_interactionItem->setZoomScale(targetScale);
}

struct SavedViewState
{
    double zoomScale;
    QPointF panPos;

    SavedViewState(WebViewportItem* item)
        : zoomScale(item->zoomScale())
        , panPos(item->panPos())
    {
    }

    SavedViewState()
        : zoomScale(-1)
    {
    }

    bool isValid() { return zoomScale >= 0; }

    void imposeTo(WebViewportItem* item) const
    {
        item->setZoomScale(zoomScale);
        item->setPanPos(panPos);
    }
};
Q_DECLARE_METATYPE(SavedViewState);  // a bit heavy weight for what this is used for

void MainView::saveFrameState(QWebFrame* frame, QWebHistoryItem* item)
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__ << frame << item;
#endif

    // FIXME: this crashes with threading for some reason
    if (m_mainWindow->settings().m_enableEngineThread)
        return;
    if (frame == webView()->page()->mainFrame())
        item->setUserData(QVariant::fromValue(SavedViewState(m_interactionItem)));
#if defined(ENABLE_LOADEVENT_DEBUG)
    else
        qDebug() << "Unknown frame at " << __PRETTY_FUNCTION__;
#endif
}

void MainView::restoreFrameState(QWebFrame* frame)
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__ << frame;
#endif

    if (frame == webView()->page()->mainFrame()) {
        QWebHistoryItem item = webView()->page()->history()->currentItem();
        QVariant value = item.userData();

        if (value.canConvert<SavedViewState>()) {
            SavedViewState s = value.value<SavedViewState>();
            if (s.isValid())
                s.imposeTo(m_interactionItem);
        }
    }
#if defined(ENABLE_LOADEVENT_DEBUG)
    else
        qDebug() << "Unknown frame at " << __PRETTY_FUNCTION__;
#endif
}

void MainView::showTiles(bool tilesOn) 
{
    m_tilesOn = tilesOn;
    QMapIterator<int, TileItem*> i(m_tileMap);
    while (i.hasNext()) {
        i.next();
        i.value()->setVisible(tilesOn);
    }
}

void MainView::tileCreated(unsigned hPos, unsigned vPos)
{
    // new tile or just inactive?
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos)))
        m_tileMap.insert(TILE_KEY(hPos, vPos), new TileItem(hPos, vPos, m_tilesOn, this));
    else
        m_tileMap.value(TILE_KEY(hPos, vPos))->setActive(true);
}

void MainView::tileRemoved(unsigned hPos, unsigned vPos)
{
    Q_ASSERT(m_tileMap.contains(TILE_KEY(hPos, vPos)));    
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos))) {
        // qDebug() << __FUNCTION__ << " didn't find tile at:" << hPos << " " <<  vPos;
        return;
    }
    m_tileMap.value(TILE_KEY(hPos, vPos))->setActive(false);
}

void MainView::tilePainted(unsigned hPos, unsigned vPos)
{
    Q_ASSERT(m_tileMap.contains(TILE_KEY(hPos, vPos)));    
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos))) {
        // qDebug() << __FUNCTION__ << " didn't find tile at:" << hPos << " " <<  vPos;
        return;
    }
    m_tileMap.value(TILE_KEY(hPos, vPos))->painted();
}

void MainView::tileCacheViewportScaleChanged()
{
    QTimer::singleShot(0, this, SLOT(resetCacheTiles()));
}

void MainView::resetCacheTiles()
{
    QMapIterator<int, TileItem*> i(m_tileMap);
    while (i.hasNext()) {
        i.next();
        delete m_tileMap.take(i.key());
    }
}

QRect MainView::progressRect()
{
    int height = m_progressBox->fontMetrics().height();
    int width = qMin(m_progressBox->fontMetrics().size(Qt::TextSingleLine, s_initialProgressText).width() + 30, 100);
    return QRect(0, scene()->sceneRect().bottomLeft().y() - (height + 3), width, height + 3);
}

#include "MainView.moc"
