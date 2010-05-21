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

#include "HomeView.h"
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>

#include "PannableTileContainer.h"
#include "TileContainerWidget.h"
#include "TileItem.h"
#include "HistoryStore.h"
#include "BookmarkStore.h"
#include "WebView.h"
#if USE_DUI
#include <DuiScene>
#endif

#include <QDebug>

const int s_maxWindows = 6;
const int s_containerMargin = 60;
const int s_maxHistoryTileNum = 20;
const int s_horizontalFlickLockThreshold = 20;
const int s_flickTimeoutThreshold = 700;

HomeView::HomeView(HomeWidgetType initialWidget, QPixmap* bckg, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : TileSelectionViewBase(TileSelectionViewBase::Home, bckg, parent, wFlags)
    , m_activeWidget(initialWidget)
    , m_bookmarkWidget(new BookmarkWidget(this, wFlags))
    , m_historyWidget(new HistoryWidget(this, wFlags))
    , m_tabWidget(new TabWidget(this, wFlags))
    , m_pannableHistoryContainer(new PannableTileContainer(this, wFlags))
    , m_pannableBookmarkContainer(new PannableTileContainer(this, wFlags))
    , m_pannableWindowSelectContainer(new PannableTileContainer(this, wFlags))
    , m_windowList(0) 
{
    m_pannableHistoryContainer->setWidget(m_historyWidget);
    m_pannableBookmarkContainer->setWidget(m_bookmarkWidget);
    m_pannableWindowSelectContainer->setWidget(m_tabWidget);
    
    m_tabWidget->setEditMode(true);

    connect(m_tabWidget, SIGNAL(closeWidget(void)), this, SLOT(closeViewSoon()));
    connect(m_bookmarkWidget, SIGNAL(closeWidget(void)), this, SLOT(closeViewSoon()));
    connect(m_historyWidget, SIGNAL(closeWidget(void)), this, SLOT(closeViewSoon()));
}

HomeView::~HomeView()
{
    delete m_tabWidget;
    delete m_pannableWindowSelectContainer;
    delete m_pannableHistoryContainer;
    delete m_pannableBookmarkContainer;
}

void HomeView::setActiveWidget(HomeWidgetType widget)
{
    if (widget == m_activeWidget)
        return;

    m_activeWidget = widget;
    int containerWidth = rect().width() / 3 - s_containerMargin;

    // repositon view
    if (m_activeWidget == WindowSelect)
        setPos(0, 0);
    else if (m_activeWidget == VisitedPages)
        setPos(QPointF(-(containerWidth - s_containerMargin), 0));
    else if (m_activeWidget == Bookmarks)
        setPos(QPointF(-(2*containerWidth - 2*s_containerMargin), 0));
}

// FIXME this should all be somewhere else. and then cancelLeftMouseButtonPress could go back to normal.
bool HomeView::filterMouseEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->type() == QEvent::GraphicsSceneMousePress) {
        m_flickTime.start();        
        m_mouseDown = true;
        m_horizontalFlickLocked = false;
        m_hDelta = 0;
        m_mousePos = e->scenePos();
    } else if (e->type() == QEvent::GraphicsSceneMouseRelease) {
        m_mouseDown = false;
        if (!m_horizontalFlickLocked)
            return false;
        moveViews();
        return true;
    } else if (e->type() == QEvent::GraphicsSceneMouseMove && m_mouseDown) {
        // panning
        QPointF delta(m_mousePos - e->scenePos());
        m_mousePos = e->scenePos();
        
        if (m_horizontalFlickLocked) {
            QPointF p(pos());
            p.setX(p.x() - delta.x());
            if (p.x() > 0)
                p.setX(0);
            else if (abs(p.x()) > size().width()/3*2)
                p.setX(-(size().width()/3*2));            
            setPos(p);
            return true;
        } else if (abs(delta.x()) >= abs(delta.y())) {
            m_hDelta+=delta.x();
            m_horizontalFlickLocked = abs(m_hDelta) > s_horizontalFlickLockThreshold;
            // let the gesture recognizer know that we are hijacking the gesture
            if (m_horizontalFlickLocked && activePannableContainer())
                activePannableContainer()->cancelLeftMouseButtonPress(QPoint());
        }
    }
    return false;
}

void HomeView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QRectF r(rect()); 
    int containerWidth = r.width() / 3 - s_containerMargin;

    // tab to the leftmost pos
    r.setWidth(containerWidth);
    m_pannableWindowSelectContainer->setGeometry(r);
    m_tabWidget->setGeometry(r);
    if (m_activeWidget == WindowSelect)
        setPos(-r.topLeft());

    // move history to the middle
    r.moveLeft(containerWidth); 
    r.setWidth(containerWidth - s_containerMargin);
    m_pannableHistoryContainer->setGeometry(r);
    m_historyWidget->setGeometry(QRectF(QPointF(0, 0), r.size()));
    if (m_activeWidget == VisitedPages)
        setPos(QPointF(-(r.x() - s_containerMargin), 0));

    // rigthmost
    r.moveLeft(2*containerWidth - s_containerMargin); 
    m_pannableBookmarkContainer->setGeometry(r);
    m_bookmarkWidget->setGeometry(QRectF(QPointF(0, 0), r.size()));
    if (m_activeWidget == Bookmarks)
        setPos(QPointF(-(r.x() - s_containerMargin), 0));
    TileSelectionViewBase::resizeEvent(event);
}

void HomeView::tileItemActivated(TileItem* item)
{
    TileSelectionViewBase::tileItemActivated(item);
    if (m_activeWidget == WindowSelect) {
        if (item->context())
            emit windowSelected((WebView*)item->context());
        else {
            emit windowCreated();
        }
    } else {
        emit pageSelected(item->urlItem()->url());
    }
}

void HomeView::tileItemClosed(TileItem* item)
{
    TileSelectionViewBase::tileItemClosed(item);
    widgetByType(m_activeWidget)->removeTile(*item);

    if (m_activeWidget == WindowSelect)
        emit windowClosed((WebView*)item->context());
}

void HomeView::tileItemEditingMode(TileItem* item)
{
    TileSelectionViewBase::tileItemEditingMode(item);

    TileBaseWidget* w = widgetByType(m_activeWidget); 
    w->setEditMode(!w->editMode());
    update();
}

void HomeView::moveViews()
{
    // swap(flick), move back, or no move at all?

    // a definite flick
    bool flick = m_flickTime.elapsed() < s_flickTimeoutThreshold;

    // check which container wins
    int containerWidth = size().width() / 3 - s_containerMargin;
    
    if (flick) {
        if ((m_activeWidget == WindowSelect && m_hDelta > 0) || (m_activeWidget == Bookmarks && m_hDelta < 0))
            m_activeWidget = VisitedPages;
        else if (m_activeWidget == VisitedPages)
            m_activeWidget = m_hDelta > 0 ? Bookmarks : WindowSelect;
    } else {    
        // at what pos it should jump to the next container
        int goOverWidth = containerWidth / 2;
        switch (abs(pos().x() / goOverWidth)) {
            default:
            case 0:
                m_activeWidget = WindowSelect;
                break;
            case 1:
            case 2:
                m_activeWidget = VisitedPages;
                break;
            case 3:
                m_activeWidget = Bookmarks;
                break;
        }
    }

    int newPos = 0;
    // rightmost >> middle >> leftmost
    if (m_activeWidget == Bookmarks)
        newPos = 2 * containerWidth - 2*s_containerMargin;
    else if (m_activeWidget == VisitedPages)
        newPos = containerWidth - s_containerMargin;

    QRectF to(geometry());
    to.moveLeft(-newPos);

    QPropertyAnimation* slideAnim = new QPropertyAnimation(this, "geometry");
    slideAnim->setStartValue(geometry());
    slideAnim->setEndValue(to);

    slideAnim->setDuration(500);
    slideAnim->setEasingCurve(flick ? QEasingCurve::OutQuad : QEasingCurve::OutExpo);
    slideAnim->start();
}

void HomeView::destroyViewItems()
{
    m_tabWidget->removeAll();
    m_historyWidget->removeAll();
    m_bookmarkWidget->removeAll();
}

void HomeView::createViewItems()
{
    // recreate?
    destroyViewItems();
    //
    createTabSelectContent();
    createHistoryContent();
    createBookmarkContent();
}

void HomeView::createBookmarkContent()
{
    const UrlList& list = BookmarkStore::instance()->list();
    for (int i = 0; i < list.size(); ++i) {
        ListTileItem* newTileItem = new ListTileItem(m_bookmarkWidget, list.at(i));
        newTileItem->setEditMode(m_bookmarkWidget->editMode());
        m_bookmarkWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    m_bookmarkWidget->layoutTiles();
}

void HomeView::createHistoryContent()
{
    ThumbnailTileItem* newTileItem = 0;
    const UrlList& list = HistoryStore::instance()->list();
    for (int i = 0; i < (s_maxHistoryTileNum - 1) && i < list.size(); ++i) {
        newTileItem = new ThumbnailTileItem(m_historyWidget, list.at(i));
        newTileItem->setEditMode(m_historyWidget->editMode());
        m_historyWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    m_historyWidget->layoutTiles();
}

void HomeView::createTabSelectContent()
{
    if (!m_windowList)
        return;

    // create tile list out of window list
    int i = 0;
    for (; i < m_windowList->size(); ++i) {
        WebView* view = m_windowList->at(i);
        bool pageAvailable = !view->url().isEmpty();
        QImage* thumbnail = 0;
    
        if (pageAvailable) {
            // get the thumbnail from history store, it'd better be there
            const UrlList& l = HistoryStore::instance()->list();
            for (int i = 0; i < l.size(); ++i) {
                if (l.at(i).url() == view->url()) {
                    thumbnail = new QImage(*(l.at(i).thumbnail()));
                    break;
                }
            }
        }
        // create a tile item with the window context set
        ThumbnailTileItem* newTileItem = new ThumbnailTileItem(m_tabWidget, UrlItem(view->url(), pageAvailable ? view->title() : "Page not loded yet", thumbnail));
        newTileItem->setEditMode(m_tabWidget->editMode());
        newTileItem->setContext(view);

        m_tabWidget->addTile(*newTileItem);
        connectItem(*newTileItem);
    }
    
    NewWindowTileItem* newTileItem = new NewWindowTileItem(m_tabWidget, UrlItem(QUrl(), "", 0));
    m_tabWidget->addTile(*newTileItem);
    connectItem(*newTileItem);
    i++;
    
    for (; i < s_maxWindows; i++)
        m_tabWidget->addTile(*(new NewWindowMarkerTileItem(m_tabWidget, UrlItem(QUrl(), "", 0))));

    m_tabWidget->layoutTiles();
}

TileBaseWidget* HomeView::widgetByType(HomeWidgetType type)
{
    if (type == WindowSelect)
        return m_tabWidget;
    else if (type == VisitedPages)
        return m_historyWidget;
    else if (type == Bookmarks)
        return m_bookmarkWidget;
    return 0;    
}

PannableTileContainer* HomeView::activePannableContainer()
{
    if (m_activeWidget == WindowSelect)
        return m_pannableWindowSelectContainer;
    else if (m_activeWidget == VisitedPages)
        return m_pannableHistoryContainer;
    else if (m_activeWidget == Bookmarks)
        return m_pannableBookmarkContainer;
    return 0;    
}

