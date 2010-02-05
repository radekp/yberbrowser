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

#include "PageView.h"
#include "MainWindow.h"
#include "WebViewportItem.h"
#include "HistoryViewportItem.h"
#include "UrlStore.h"
#include "Settings.h"

static const unsigned s_progressBckgColor = 0xA0C6F3;
static const unsigned s_progressTextColor = 0x185488;
static QString s_initialProgressText("Loading...");

// TODO:
// detect when user interaction has been done and do
// m_state = Interaction;

// to debug load events etc:
// make DEFINES+=-DENABLE_LOADEVENT_DEBUG
// this is needed in order to make load/back/forward etc
// as fast as possible, doing as little zooming changes as possible
// qt api is not very clear on the signal order. once the aspects
// of load signal order have been cleared, remove these ifdefs

PageView::PageView(MainWindow* window)
    : QGraphicsView(window)
    , m_mainWindow(window)
    , m_pageViewportItem(0)
    , m_historyViewportItem(0)
    , m_state(InitialLoad)
    , m_webView(0)
    , m_progressBox(0)
{
    if (Settings::instance()->useGL())  {
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

PageView::~PageView()
{
    // delete history only when it is not active
    if (scene()->activeWindow() != m_historyViewportItem)
        delete m_historyViewportItem;
    else 
        delete m_pageViewportItem;
    // todo: figure out ownership
    delete m_progressBox;
}

void PageView::init()
{
    m_historyViewportItem = new HistoryViewportItem(*this);
    scene()->addItem(m_historyViewportItem);
    m_historyViewportItem->setZValue(100);
    scene()->setActiveWindow(m_historyViewportItem);
    connect(m_historyViewportItem, SIGNAL(hideHistory()), this, SLOT(disableHistoryView()));
}

void PageView::setWebView(WebView* webViewItem)
{
    if (webViewItem) {
        if (!m_pageViewportItem) {
            Q_ASSERT(scene());
            m_pageViewportItem = new WebViewportItem();
            scene()->addItem(m_pageViewportItem);
        }
        m_pageViewportItem->setWebView(m_webView = webViewItem);
        installSignalHandlers();
        updateSize();
    } else {
        m_pageViewportItem->setParent(0);
        delete m_pageViewportItem;
        m_pageViewportItem = 0;
    }
}

void PageView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateSize();
}

void PageView::updateSize()
{
    setUpdatesEnabled(false);
    if (!m_pageViewportItem)
        return;

    QRectF rect(QPointF(0, 0), size());
    scene()->setSceneRect(rect);

    m_pageViewportItem->setGeometry(rect);
    updateZoomScaleToPageWidth();
    setUpdatesEnabled(true);

    m_historyViewportItem->setGeometry(rect);

    update();
}

WebView* PageView::webView()
{
    return m_webView;
}

void PageView::installSignalHandlers()
{
    connect(webView()->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), this, SLOT(resetState()));
    connect(webView()->page()->mainFrame(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(contentsSizeChanged(const QSize&)));
    connect(webView()->page(),SIGNAL(saveFrameStateRequested(QWebFrame*,QWebHistoryItem*)), SLOT(saveFrameState(QWebFrame*,QWebHistoryItem*)));
    connect(webView()->page(),SIGNAL(restoreFrameStateRequested(QWebFrame*)), SLOT(restoreFrameState(QWebFrame*)));
    connect(webView(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(webView(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(webView(), SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
}

void PageView::resetState()
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    m_state = InitialLoad;
    if (m_pageViewportItem) {
        // zoom will be updated in contentsSizeChanged
        m_pageViewportItem->resetState(false);
    }

    setUpdatesEnabled(true);
    update();
}

void PageView::loadStarted()
{
    if (m_historyViewportItem->isActive())
        toggleHistory();
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

void PageView::loadProgress(int progress)
{
    // todo: find out this magic 10% thing
    if (progress <= 10)
        return;
    m_progressBox->setText(QString::number(progress) + "%");
}

void PageView::loadFinished(bool)
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__;
#endif
    m_progressBox->hide();
    disconnect(scene(), SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(sceneRectChanged(const QRectF&)));

    if (m_state == InitialLoad)
        m_state = Interaction;
}

void PageView::sceneRectChanged(const QRectF& /*rect*/)
{
    m_progressBox->setGeometry(progressRect());
}

void PageView::contentsSizeChanged(const QSize&)
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

void PageView::updateZoomScaleToPageWidth()
{
    if (!m_pageViewportItem)
        return;

    QSize contentsSize = webView()->page()->mainFrame()->contentsSize();
    qreal targetScale = 1.;
    if (contentsSize.width()) {
        targetScale = static_cast<qreal>(m_pageViewportItem->size().width()) / contentsSize.width();
    }
    m_pageViewportItem->setZoomScale(targetScale);
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

void PageView::saveFrameState(QWebFrame* frame, QWebHistoryItem* item)
{
#if defined(ENABLE_LOADEVENT_DEBUG)
    qDebug() << __FUNCTION__ << frame << item;
#endif

    // FIXME: this crashes with threading for some reason
    if (Settings::instance()->engineThreadEnabled())
        return;
    if (frame == webView()->page()->mainFrame())
        item->setUserData(QVariant::fromValue(SavedViewState(m_pageViewportItem)));
#if defined(ENABLE_LOADEVENT_DEBUG)
    else
        qDebug() << "Unknown frame at " << __PRETTY_FUNCTION__;
#endif
}

void PageView::restoreFrameState(QWebFrame* frame)
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
                s.imposeTo(m_pageViewportItem);
        }
    }
#if defined(ENABLE_LOADEVENT_DEBUG)
    else
        qDebug() << "Unknown frame at " << __PRETTY_FUNCTION__;
#endif
}

QRect PageView::progressRect()
{
    int height = m_progressBox->fontMetrics().height();
    int width = qMin(m_progressBox->fontMetrics().size(Qt::TextSingleLine, s_initialProgressText).width() + 30, 100);
    return QRect(0, scene()->sceneRect().bottomLeft().y() - (height + 3), width, height + 3);
}

void PageView::toggleHistory()
{
    // hack
    if (m_historyViewportItem->zValue() == -1) {
        scene()->addItem(m_historyViewportItem);
        m_historyViewportItem->setZValue(100);
        scene()->setActiveWindow(m_historyViewportItem);
    }
    m_historyViewportItem->toggleHistory();
}

// callback from historyview when outbound animation finished
void PageView::disableHistoryView()
{
    scene()->removeItem(m_historyViewportItem);
    m_historyViewportItem->setZValue(-1);
    scene()->setActiveWindow(m_pageViewportItem);
}

