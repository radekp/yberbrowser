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
#include <QtGlobal>

#include "MainView.h"
#include "WebViewportItem.h"


// TODO:
// detect when user interaction has been done and do
// m_state = Interaction;

MainView::MainView(QWidget* parent, Settings settings)
    : QGraphicsView(parent)
    , m_interactionItem(0)
    , m_state(InitialLoad)
{
    if (settings.m_useGL)
        setViewport(new QGLWidget);

    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

MainView::~MainView()
{
    delete m_interactionItem;
}

void MainView::setWebView(QGraphicsWebView* webViewItem)
{
    if (webViewItem) {
        if (!m_interactionItem) {
            Q_ASSERT(scene());
            m_interactionItem = new WebViewportItem();
            scene()->addItem(m_interactionItem);
            scene()->setActiveWindow(m_interactionItem);
        }
        m_interactionItem->setWebView(webViewItem);
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
    if (!m_interactionItem)
        return;

    QRectF rect(QPointF(0, 0), size());
    scene()->setSceneRect(rect);

    m_interactionItem->setGeometry(rect);
    updateZoomScaleToPageWidth();
}

QGraphicsWebView* MainView::webView()
{
    return m_interactionItem->webView();
}

void MainView::installSignalHandlers()
{
    connect(webView()->page()->mainFrame(), SIGNAL(initialLayoutCompleted()), this, SLOT(resetState()));
    connect(webView()->page()->mainFrame(), SIGNAL(contentsSizeChanged(const QSize &)), this, SLOT(contentsSizeChanged(const QSize&)));
    connect(webView(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
}

void MainView::resetState()
{
    m_state = InitialLoad;
    if (m_interactionItem) {
        m_interactionItem->resetState(true);
    }
}

void MainView::loadFinished(bool)
{
    if (m_state == InitialLoad)
        m_state = Interaction;
}

void MainView::contentsSizeChanged(const QSize& sz)
{
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
    m_interactionItem->setZoomScale(targetScale, true);
}

