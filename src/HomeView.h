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

#ifndef HomeView_h_
#define HomeView_h_

#include "TileSelectionViewBase.h"
#include <QTime>

class HistoryWidget;
class BookmarkWidget;
class TabWidget;
class PannableTileContainer;
class TileItem;
class QGraphicsSceneMouseEvent;
class WebView;
class TileBaseWidget;

class HomeView : public TileSelectionViewBase {
    Q_OBJECT
public:
    enum HomeWidgetType {
        WindowSelect,
        VisitedPages,
        Bookmarks
    };
   
    HomeView(HomeWidgetType initialWidget, QPixmap* bckg, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HomeView();
    
    void setWindowList(QList<WebView*>& windowList) { m_windowList = &windowList; }
    HomeWidgetType activeWidget() const { return m_activeWidget; }
    void setActiveWidget(HomeWidgetType widget);
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    // FIXME temp hack until event handling is fixed
    bool filterMouseEvent(QGraphicsSceneMouseEvent*);

Q_SIGNALS:
    void pageSelected(const QUrl&);
    void windowSelected(WebView* webView);
    void windowClosed(WebView* webView);
    void windowCreated();

private Q_SLOTS:
    void tileItemActivated(TileItem*);
    void tileItemClosed(TileItem*);
    void tileItemEditingMode(TileItem*);

private:
    void moveViews();

    void createViewItems();
    void destroyViewItems();

    void createBookmarkContent();
    void createHistoryContent();
    void createTabSelectContent();

    TileBaseWidget* widgetByType(HomeWidgetType type);
    void resetContainerSize();

private:
    HomeWidgetType m_activeWidget;
    BookmarkWidget* m_bookmarkWidget;
    HistoryWidget* m_historyWidget;
    TabWidget* m_tabWidget;
    PannableTileContainer* m_pannableHistoryContainer;
    PannableTileContainer* m_pannableBookmarkContainer;
    PannableTileContainer* m_pannableWindowSelectContainer;
    QList<WebView*>* m_windowList;

    // FIXME these should go to a gesture recognizer
    QTime m_flickTime;
    bool m_horizontalFlickLocked;
    bool m_mouseDown;
    QPointF m_mousePos;
    int m_hDelta;
};

#endif
