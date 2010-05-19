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
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsSimpleTextItem>
#include <QPen>

const int s_viewMargin = 40;
const int s_tileMargin = 40;
const int s_titleVMargin = 10;
const int s_bookmarksTileHeight = 70;
const int s_searchItemTileHeight = 60;

TileBaseWidget::TileBaseWidget(const QString& title, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_titleItem(new QGraphicsSimpleTextItem(title, this))
    , m_slideAnimationGroup(0)
    , m_title(title)
    , m_editMode(false)
{
    m_titleItem->setFont(QFont("Times", 30));
    m_titleItem->setPen(QPen(Qt::white));
    m_titleItem->setBrush(QBrush(Qt::white));
    m_titleItem->setPos(s_viewMargin, s_titleVMargin);
}

TileBaseWidget::~TileBaseWidget()
{
    removeAll();
    delete m_slideAnimationGroup;
}

QSize TileBaseWidget::doLayoutTiles(const QRectF& rect_, int hTileNum, int vTileNum, int marginX, int marginY, bool fixed)
{
    if (!m_tileList.size())
        return QSize(0, 0) ;

    int width = rect_.width() - (hTileNum + 1)*marginX;
    int height = rect_.height() - (vTileNum + 1)*marginY;

    int tileWidth = width / hTileNum;
    int tileHeight = height / vTileNum; 

    int tileCount = fixed ? qMin(hTileNum * vTileNum, m_tileList.size()) : m_tileList.size();
    int y = rect_.top() + s_viewMargin - tileHeight;
    int x = rect_.left() + marginX;
    int i = 0;
    for (; i < tileCount; ++i) {
        if (i%hTileNum == 0) {
            y+=(tileHeight + marginY);
            x = rect_.left() + marginX;
        }
        m_tileList.at(i)->setRect(QRectF(x, y, tileWidth, tileHeight));
        x+=(tileWidth + marginX);
    }

    if (fixed) {
        // leftovers are hidden, lined up after the last item
        for (int j = i; j < m_tileList.size(); ++j) {
            m_tileList.at(j)->setRect(QRectF(x, y, tileWidth, tileHeight));
            x+=(tileWidth + marginX);
            m_tileList.at(j)->hide();
        }
    }
    return QSize(tileWidth * hTileNum, y + tileHeight);
}

void TileBaseWidget::addTile(TileItem& newItem)
{
    m_tileList.append(&newItem);
    newItem.setEditMode(m_editMode);
}

void TileBaseWidget::removeTile(TileItem& removed)
{
    if (!m_slideAnimationGroup)
        m_slideAnimationGroup = new QParallelAnimationGroup();

    int hiddenIndex = -1;
    m_slideAnimationGroup->clear();
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
    m_slideAnimationGroup->start(QAbstractAnimation::KeepWhenStopped);
    // FIXME check if container needs to be resized
}

void TileBaseWidget::removeAll()
{
    for (int i = m_tileList.size() - 1; i >= 0; --i)
        delete m_tileList.takeAt(i);
}

void TileBaseWidget::setEditMode(bool on) 
{ 
    m_editMode = on;
    for (int i = 0; i < m_tileList.size(); ++i)
        m_tileList.at(i)->setEditMode(on);
}

void TileBaseWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    emit closeWidget();
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
    // FIXME: this is landscape oriented. check aspect ratio
    QRectF r(rect());
    r.setHeight(r.height() - s_viewMargin);
    bool landscape = parentWidget()->size().width() > parentWidget()->size().height();
    int hTileNum = landscape ? 3 : 2;
    int vTileNum = landscape ? 2 : 3;
    doLayoutTiles(r, hTileNum, vTileNum, s_tileMargin, s_tileMargin, true);
}

void TabWidget::removeTile(TileItem& removed)
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
    delete removed.urlItem();
    TileBaseWidget::removeTile(removed);
}

void TabWidget::removeAll()
{
    // url list is created here (out of window list) unlike in other views, like history items.
    for (int i = m_tileList.size() - 1; i >= 0; --i)
        delete m_tileList.at(i)->urlItem();
    TileBaseWidget::removeAll();
}

// history
HistoryWidget::HistoryWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) 
    : TileBaseWidget("Top Sites", parent, wFlags) 
{
}

void HistoryWidget::layoutTiles()
{
    // the height of the view is unknow until we layout the tiles
    QRectF r(rect());
    r.setHeight(parentWidget()->size().height() - s_viewMargin);
    bool landscape = parentWidget()->size().width() > parentWidget()->size().height();
    int hTileNum = landscape ? 3 : 2;
    int vTileNum = landscape ? 2 : 3;
    setMinimumHeight(doLayoutTiles(r, hTileNum, vTileNum, s_tileMargin, s_tileMargin).height()); 
}

// bookmarks
BookmarkWidget::BookmarkWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) 
    : TileBaseWidget("Bookmarks", parent, wFlags) 
{
}

void BookmarkWidget::layoutTiles()
{
    // the height of the view is unknow until we layout the tiles
    QRectF r(rect());
    r.setTop(s_viewMargin);
    setMinimumHeight(doLayoutTiles(r, 1, r.height()/s_bookmarksTileHeight, s_tileMargin, 0).height());
}

// url filter popup
PopupWidget::PopupWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags) 
    : TileBaseWidget("Search result", parent, wFlags) 
{
}

void PopupWidget::layoutTiles()
{
    QRectF r(rect());
    r.setTop(s_viewMargin);
    setMinimumHeight(doLayoutTiles(r, 1, r.height()/s_searchItemTileHeight, s_tileMargin, -1).height());
}


