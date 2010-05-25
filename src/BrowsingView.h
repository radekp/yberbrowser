/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef BrowsingView_h
#define BrowsingView_h

#include "yberconfig.h"
#include "TileSelectionViewBase.h"
#include "HomeView.h"

#if USE_DUI
#include <DuiApplicationPage>

typedef DuiApplicationPage BrowsingViewBase;
typedef DuiWidget YberWidget;
class DuiTextEdit;
#else
#include <QGraphicsWidget>
#include <QUrl>
typedef QGraphicsWidget BrowsingViewBase;
typedef QGraphicsWidget YberWidget;

class QMenuBar;
typedef QMenuBar MenuBar;
#endif

class WebView;
class YberApplication;
class WebViewportItem;
class PannableViewport;
class ApplicationWindow;
class AutoScrollTest;
class PopupView;
class QPixmap;
class ToolbarWidget;
class QGraphicsProxyWidget;

class BrowsingView : public BrowsingViewBase
{
    Q_OBJECT
public:
    BrowsingView(YberApplication&, QGraphicsItem* parent = 0);
    ~BrowsingView();

#if !USE_DUI
    void appear(ApplicationWindow* window);
    YberWidget* centralWidget()  { return m_centralWidget; }
    PannableViewport* pannableViewport() { return m_browsingViewport; }

    void createHomeView(HomeView::HomeWidgetType type);

#endif
public Q_SLOTS:
    void load(const QUrl&);
    WebView* newWindow();
    void destroyWindow(WebView* webView);
    void setActiveWindow(WebView* webView);
#if !USE_DUI
    void setTitle(const QString&);
#endif

protected:
    void resizeEvent(QGraphicsSceneResizeEvent* event);

protected Q_SLOTS:
    void addBookmark();
    void stopLoad();
    void pageBack();
    void urlEditfocusChanged(bool);
    void urlEditingFinished(const QString& url);
    void urlChanged(const QUrl& url);

    void loadStarted();
    void progressChanged(int);
    void loadFinished(bool success);

    void toggleFullScreen();

    void prepareForResize();

    void startAutoScrollTest();
    void finishedAutoScrollTest();

    void windowSelected(WebView* webView);
    void windowClosed(WebView* webView);
    void windowCreated();
    void deleteHomeView();

private:
    Q_DISABLE_COPY(BrowsingView);
    void connectWebViewSignals(WebView* currentView, WebView* oldView);
    void updateHistoryStore(bool successLoad);
    QPixmap* webviewSnapshot();

#if !USE_DUI
    QMenuBar* createMenu(QWidget* parent);
#endif

#if !USE_DUI
    YberWidget* m_centralWidget;
#endif

    WebView* m_activeWebView;
    PannableViewport* m_browsingViewport;
    WebViewportItem* m_webInteractionProxy;
    QSizeF m_sizeBeforeResize;
    AutoScrollTest* m_autoScrollTest;
    QList<WebView*> m_windowList;
    HomeView* m_homeView;
    HomeView::HomeWidgetType m_initialHomeWidget;
    ToolbarWidget* m_toolbarWidget;
    ApplicationWindow* m_appWin;
};


#endif
