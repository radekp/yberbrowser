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

#ifndef PageView_h_
#define PageView_h_

#include <QGraphicsView>

#include "MainWindow.h"

class QGraphicsWidget;
class QResizeEvent;
class MainWindow;
class WebViewportItem;
class HistoryViewportItem;
class TileItem;
class QLabel;
class HistoryItem;

class PageView : public QGraphicsView {
    Q_OBJECT

public:
    PageView(MainWindow* parent);
    ~PageView();

    void init();
    void setWebView(WebView* widget);
    WebView* webView();

    void resizeEvent(QResizeEvent* event);

    WebViewportItem* interactionItem() const { return m_pageViewportItem; }

    MainWindow* mainWindow() const { return m_mainWindow; }

    void showTiles(bool tilesOn);

public Q_SLOTS:
    void toggleHistory();

protected Q_SLOTS:
    void resetState();
    void contentsSizeChanged(const QSize &size);
    void loadFinished(bool);
    void loadStarted();
    void loadProgress(int);
    void sceneRectChanged(const QRectF&);
    void saveFrameState(QWebFrame* frame, QWebHistoryItem* item);
    void restoreFrameState(QWebFrame* frame);
    void tileCreated(unsigned hPos, unsigned vPos);
    void tileRemoved(unsigned hPos, unsigned vPos);
    void tilePainted(unsigned hPos, unsigned vPos);
    void tileCacheViewportScaleChanged();
    void resetCacheTiles();
    void disableHistoryView();

private:
    void updateSize();
    void installSignalHandlers();
    void updateZoomScaleToPageWidth();
    QRect progressRect();

    enum State {
        InitialLoad,
        Interaction
    };

private:
    MainWindow* m_mainWindow;
    WebViewportItem* m_pageViewportItem;
    HistoryViewportItem* m_historyViewportItem;
    State m_state;
    WebView* m_webView;
    bool m_tilesOn;
    QMap<int, TileItem*> m_tileMap;  
    QLabel* m_progressBox;
};

#endif