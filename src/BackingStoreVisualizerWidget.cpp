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

#include "BackingStoreVisualizerWidget.h"

#include <qgraphicswebview.h>
#include <QTimer>
#include <QDebug>

static const unsigned s_tileSize = 35;

#define TILE_KEY(x,y) (x << 16 | y)

class BackingStoreTileItem : public QObject {
    Q_OBJECT
public:
    BackingStoreTileItem(unsigned hPos, unsigned vPos, QGraphicsItem* parent);
    ~BackingStoreTileItem();

    bool isActive() const;
    void setActive(bool active);
    void painted();

private Q_SLOTS:
    void paintBlinkEnd();

private:
    unsigned           m_vPos;
    unsigned           m_hPos;
    bool               m_active;
    unsigned           m_beingPainted;
    QGraphicsRectItem* m_rectItem;
};

BackingStoreTileItem::BackingStoreTileItem(unsigned hPos, unsigned vPos, QGraphicsItem* parent)
    : m_vPos(vPos)
    , m_hPos(hPos)
    , m_active(false)
    , m_beingPainted(0)
    , m_rectItem(new QGraphicsRectItem(hPos*s_tileSize, vPos*s_tileSize, s_tileSize, s_tileSize, parent))
{
    setActive(true);
}

BackingStoreTileItem::~BackingStoreTileItem()
{
    delete m_rectItem;
}

bool BackingStoreTileItem::isActive() const
{
    return m_active;
}

void BackingStoreTileItem::setActive(bool active)
{
    if (active && m_active)
        qDebug() << "duplicate tile at:" << m_hPos << " " <<  m_vPos;

    m_active = active;
    m_rectItem->setBrush(QBrush(active?Qt::cyan:Qt::gray));
    m_rectItem->setOpacity(0.4);
}

void BackingStoreTileItem::painted()
{
    if (!m_rectItem->isVisible() || !m_active || m_beingPainted)
        return;
    m_beingPainted = 1;
    m_rectItem->setBrush(QBrush(Qt::red));
    QTimer::singleShot(1000, this, SLOT(paintBlinkEnd()));
}

void BackingStoreTileItem::paintBlinkEnd()
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

BackingStoreVisualizerWidget::BackingStoreVisualizerWidget(QGraphicsWebView* webView, QGraphicsItem* parent)
    : QGraphicsWidget(parent)
    , m_webView(webView)
{
    connectSignals();
}

BackingStoreVisualizerWidget::~BackingStoreVisualizerWidget()
{
    disconnectSignals();
    resetCacheTiles();
}


void BackingStoreVisualizerWidget::connectSignals()
{
    connect(m_webView->page(),SIGNAL(tileCreated(unsigned, unsigned)), this, SLOT(tileCreated(unsigned, unsigned)));
    connect(m_webView->page(),SIGNAL(tileRemoved(unsigned, unsigned)), this, SLOT(tileRemoved(unsigned, unsigned)));
    connect(m_webView->page(),SIGNAL(tilePainted(unsigned, unsigned)), this, SLOT(tilePainted(unsigned, unsigned)));
    connect(m_webView->page(),SIGNAL(tileCacheViewportScaleChanged()), this, SLOT(tileCacheViewportScaleChanged()));
}

void BackingStoreVisualizerWidget::disconnectSignals()
{
    disconnect(m_webView->page(),SIGNAL(tileCreated(unsigned, unsigned)), this, SLOT(tileCreated(unsigned, unsigned)));
    disconnect(m_webView->page(),SIGNAL(tileRemoved(unsigned, unsigned)), this, SLOT(tileRemoved(unsigned, unsigned)));
    disconnect(m_webView->page(),SIGNAL(tilePainted(unsigned, unsigned)), this, SLOT(tilePainted(unsigned, unsigned)));
    disconnect(m_webView->page(),SIGNAL(tileCacheViewportScaleChanged()), this, SLOT(tileCacheViewportScaleChanged()));
}


void BackingStoreVisualizerWidget::tileCreated(unsigned hPos, unsigned vPos)
{
    // new tile or just inactive?
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos)))
        m_tileMap.insert(TILE_KEY(hPos, vPos), new BackingStoreTileItem(hPos, vPos, this));
    else
        m_tileMap.value(TILE_KEY(hPos, vPos))->setActive(true);
}

void BackingStoreVisualizerWidget::tileRemoved(unsigned hPos, unsigned vPos)
{
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos)))
        return;
    m_tileMap.value(TILE_KEY(hPos, vPos))->setActive(false);
}

void BackingStoreVisualizerWidget::tilePainted(unsigned hPos, unsigned vPos)
{
    if (!m_tileMap.contains(TILE_KEY(hPos, vPos)))
        tileCreated(hPos, vPos);
    m_tileMap.value(TILE_KEY(hPos, vPos))->painted();
}

void BackingStoreVisualizerWidget::tileCacheViewportScaleChanged()
{
    QTimer::singleShot(0, this, SLOT(resetCacheTiles()));
}

void BackingStoreVisualizerWidget::resetCacheTiles()
{
    QMapIterator<int, BackingStoreTileItem*> i(m_tileMap);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    m_tileMap.clear();
}

#include "BackingStoreVisualizerWidget.moc"
