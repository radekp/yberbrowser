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

#include "TileContainerWidget.h"
#include "BookmarkStore.h"
#include "HistoryStore.h"
#include "FontFactory.h"
#include "ToolbarWidget.h"

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QPen>
#include <QFontMetrics>
#include <QDebug>

#ifdef Q_OS_SYMBIAN
const int s_tileMargin = 25;
const int s_tileTopMargin = 5;
const int s_titleVMargin = 5;
const int s_bookmarksTileHeight = 60;
const int s_searchItemTileHeight = 60;
#else
const int s_tileMargin = 25;
const int s_tileTopMargin = 10;
const int s_titleVMargin = 10;
const int s_bookmarksTileHeight = 70;
const int s_searchItemTileHeight = 60;
#endif
const int s_containerYBottomMargin = 10;

TileBaseWidget::TileBaseWidget(const QString& title, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_title(title)
    , m_slideAnimationGroup(0)
    , m_editMode(false)
    , m_moved(false)
{
}

TileBaseWidget::~TileBaseWidget()
{
    delete m_slideAnimationGroup;
    removeAll();
}

void TileBaseWidget::addTile(TileItem& newItem)
{
    m_tileList.append(&newItem);
}

void TileBaseWidget::removeTile(const TileItem& removed)
{
    if (!m_slideAnimationGroup)
        m_slideAnimationGroup = new QParallelAnimationGroup();
    m_slideAnimationGroup->clear();

    int hiddenIndex = -1;
    for (int i = 0; i < m_tileList.size(); ++i) {
        if (m_tileList.at(i) == &removed) {
            for (int j = m_tileList.size() - 1; j > i; --j) {
                if (!m_tileList.at(j)->isVisible())
                    hiddenIndex = j;
                if (!m_tileList.at(j)->fixed())
                    addMoveAnimation(*m_tileList.at(j), (j - i) * 50, m_tileList[j]->rect().topLeft(), m_tileList[j-1]->rect().topLeft());
            }
            delete m_tileList.takeAt(i);
            break;
        }
    }
    // hidden item to appear?
    if (hiddenIndex > -1)    
        m_tileList.at(hiddenIndex - 1)->show();
    connect(m_slideAnimationGroup, SIGNAL(finished()), SLOT(adjustContainerHeight()));
    m_slideAnimationGroup->start();
}

void TileBaseWidget::removeAll()
{
    for (int i = m_tileList.size() - 1; i >= 0; --i)
        delete m_tileList.takeAt(i);
}

bool TileBaseWidget::contains(TileItem& item)
{
    return m_tileList.contains(&item);
}

void TileBaseWidget::setEditMode(bool on) 
{ 
    m_editMode = on;
    for (int i = 0; i < m_tileList.size(); ++i)
        m_tileList.at(i)->setEditMode(on);
}

int TileBaseWidget::titleVMargin() 
{
    return s_titleVMargin + ToolbarWidget::height();
}

int TileBaseWidget::tileTopVMargin() 
{
    return titleVMargin() + QFontMetrics(FontFactory::instance()->big()).height() + s_tileTopMargin;
}

QSize TileBaseWidget::doLayoutTiles(const QRectF& rect_, int hTileNum, int vTileNum, int marginX, int marginY, int fixedItemWidth, int fixedItemHeight)
{
    if (!m_tileList.size())
        return QSize(0, 0) ;

    m_titleRect.setLeft(rect().left() + marginX);
    m_titleRect.setTop(titleVMargin());
    m_titleRect.setHeight(QFontMetrics(FontFactory::instance()->big()).height());
    m_titleRect.setWidth(rect_.width());    

    int y = rect_.top() + marginY;
    int x = rect_.left() + marginX;

    int tileWidth = fixedItemWidth == -1 ? (rect_.width() - (hTileNum + 1)*marginX) / hTileNum : fixedItemWidth;
    int tileHeight = fixedItemHeight == -1 ?(rect_.height() - (vTileNum)*marginY) / vTileNum : fixedItemHeight;
    int tileCount = m_tileList.size();
    int i = 0;
    for (; i < tileCount; ++i) {
        if (i >= hTileNum && i%hTileNum == 0) {
            y+=(tileHeight + marginY);
            x = rect_.left() + marginX;
        }
        m_tileList.at(i)->setRect(QRectF(x, y, tileWidth, tileHeight));
        x+=(tileWidth + marginX);
    }
    return QSize(tileWidth * hTileNum, y + tileHeight);
}

void TileBaseWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->setFont(FontFactory::instance()->big());
    painter->setPen(QColor(Qt::white));
    painter->drawText(m_titleRect, Qt::AlignLeft|Qt::AlignVCenter, m_title);
}

void TileBaseWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    emit closeWidget();
}

void TileBaseWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
}

void TileBaseWidget::addMoveAnimation(TileItem& item, int delay, const QPointF& oldPos, const QPointF& newPos)
{
    // animate all the way down to the current window
    QPropertyAnimation* moveAnim = new QPropertyAnimation(&item, "tilePos");
    moveAnim->setDuration(500 + delay);

    moveAnim->setStartValue(oldPos);
    moveAnim->setEndValue(newPos);

    moveAnim->setEasingCurve(QEasingCurve::OutBack);
    m_slideAnimationGroup->addAnimation(moveAnim);
}

void TileBaseWidget::adjustContainerHeight()
{
    if (m_tileList.size() == 0)
        return;
    // check if container needs to be resized
    int lastRectBottom = m_tileList.at(m_tileList.size()-1)->rect().bottom() + s_containerYBottomMargin;
    if (rect().height() > lastRectBottom) {
        setMinimumHeight(lastRectBottom);
        resize(QSizeF(size().width(), lastRectBottom));
    }
}

// subclasses
// ##########

// window select
TabWidget::TabWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) 
    : TileBaseWidget("Tabs", parent, wFlags) 
{
}

void TabWidget::layoutTiles()
{
    // 6 tabs max atm
    bool landscape = parentWidget()->size().width() > parentWidget()->size().height();
    QRectF r(rect());
    // FIXME work out some proportional thing here as this 2xmargin works 
    // only on specific resolutions
    int marginY = landscape ? s_tileMargin : 2*s_tileMargin;
    r.setTop(r.top() + tileTopVMargin() - marginY);
    int hTileNum = landscape ? 3 : 2;
    int vTileNum = landscape ? 2 : 3;
    doLayoutTiles(r, hTileNum, vTileNum, s_tileMargin, marginY);
}

void TabWidget::removeTile(const TileItem& removed)
{
    // FIXME: when tab is full, fake items dont work
    // insert a fake marker item in place
    NewWindowMarkerTileItem* emptyItem = new NewWindowMarkerTileItem(this, *(new UrlItem(QUrl(), "", 0)));
    for (int i = 0; i < m_tileList.size(); ++i) {
        if (m_tileList.at(i)->fixed()) {
            emptyItem->setRect(m_tileList.at(i-1)->rect());
            m_tileList.insert(i, emptyItem);
            break;
        }            
    }
    // url list is created here (out of window list) unlike in other views, like history items.
    TileBaseWidget::removeTile(removed);
}

// history
HistoryWidget::HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) 
    : TileBaseWidget("Top Sites", parent, wFlags) 
{
}

void HistoryWidget::removeTile(const TileItem& removed)
{
    HistoryStore::instance()->remove(removed.urlItem()->url());
    TileBaseWidget::removeTile(removed);
}

void HistoryWidget::layoutTiles()
{
    // the height of the view is unknow until we layout the tiles
    QRectF r(rect());
    r.setTop(r.top() + tileTopVMargin() - s_tileMargin);
    bool landscape = parentWidget()->size().width() > parentWidget()->size().height();
    int hTileNum = landscape ? 3 : 1;
    int vTileNum = landscape ? 2 : 4;
    // show some overlapping content
    if (!landscape)
        r.setHeight(r.height() + 30);
    // add toolbarheight to make sure tiles are always visible
    setMinimumHeight(doLayoutTiles(r, hTileNum, vTileNum, landscape ? s_tileMargin : 2*s_tileMargin, s_tileMargin).height() + s_containerYBottomMargin); 
}

// bookmarks
BookmarkWidget::BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) 
    : TileBaseWidget("Bookmarks", parent, wFlags) 
{
}

void BookmarkWidget::removeTile(const TileItem& removed)
{
    BookmarkStore::instance()->remove(removed.urlItem()->url());
    TileBaseWidget::removeTile(removed);
}

void BookmarkWidget::layoutTiles()
{
    // the height of the view is unknow until we layout the tiles
    QRectF r(rect());
    r.setTop(r.top() + tileTopVMargin());
    // add toolbarheight to make sure tiles are always visible
    setMinimumHeight(doLayoutTiles(r, 1, -1, s_tileMargin, -1, -1, s_bookmarksTileHeight).height() + s_containerYBottomMargin);
}

// url filter popup
PopupWidget::PopupWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) 
    : TileBaseWidget(QString(), parent, wFlags) 
{
}

void PopupWidget::removeTile(const TileItem& removed)
{
    // FIXME should be able to know where the urlitem belongs to
    // instead of blindly trying to delete it from both stores
    BookmarkStore::instance()->remove(removed.urlItem()->url());
    HistoryStore::instance()->remove(removed.urlItem()->url());
    TileBaseWidget::removeTile(removed);
}

void PopupWidget::layoutTiles()
{
    QRectF r(rect());
    r.setTop(r.top() + ToolbarWidget::height());
    setMinimumHeight(doLayoutTiles(r, 1, -1, s_tileMargin, -1, -1, s_searchItemTileHeight).height());
}


