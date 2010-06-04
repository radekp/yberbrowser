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

#include "TileSelectionViewBase.h"
#include "ApplicationWindow.h"
#include "TileItem.h"

#include <QGraphicsPixmapItem>
#include <QTimer>

TileSelectionViewBase::TileSelectionViewBase(ViewType type, QPixmap* bckg, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_bckg(bckg ? new QGraphicsPixmapItem(*bckg, this) : 0)
    , m_type(type)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    if (m_bckg)
        m_bckg->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

TileSelectionViewBase::~TileSelectionViewBase()
{
    delete m_bckg;
}

void TileSelectionViewBase::setGeometry( const QRectF &r)
{
    QGraphicsWidget::setGeometry(r);
    if (m_bckg)
        m_bckg->setPos(-geometry().topLeft());
}

void TileSelectionViewBase::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);
    if (m_bckg)
        m_bckg->setPos(-pos());
    updateContent();
}

void TileSelectionViewBase::updateBackground(QPixmap* bckg)
{
    if (bckg) {
        if (!m_bckg)
            m_bckg = new QGraphicsPixmapItem(this);
        m_bckg->setPixmap(*bckg);
        m_bckg->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    } else {
        delete m_bckg;
        m_bckg = 0;
    }
    update();
}

void TileSelectionViewBase::updateContent()
{
    resetContainerSize();
    destroyViewItems();
    createViewItems();
}

void TileSelectionViewBase::appear()
{
    // bckg pos is misbehaving on device (n900), need to do an extra setPos here
    if (m_bckg)
        m_bckg->setPos(-pos());
}

void TileSelectionViewBase::disappear()
{
    destroyViewItems();
}

void TileSelectionViewBase::closeView()
{
    emit viewDismissed();
}

void TileSelectionViewBase::closeViewSoon()
{
    // FIXME find out why sync view close crashes
    QTimer::singleShot(0, this, SLOT(closeView()));
}

void TileSelectionViewBase::connectItem(TileItem& item)
{
    connect(&item, SIGNAL(itemActivated(TileItem*)), this, SLOT(tileItemActivated(TileItem*)));
    connect(&item, SIGNAL(itemClosed(TileItem*)), this, SLOT(tileItemClosed(TileItem*)));
    connect(&item, SIGNAL(itemEditingMode(TileItem*)), this, SLOT(tileItemEditingMode(TileItem*)));
}

