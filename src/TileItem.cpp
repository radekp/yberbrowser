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

#include "TileItem.h"
#include "FontFactory.h"

#include <QGraphicsWidget>
#include <QPainter>
#include <QtGlobal>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QDebug>

const int s_hTextMargin = 10;
const int s_longpressTimeoutThreshold = 400;
const int s_closeIconSize = 48;
const int s_closeIconClickAreaMargin = 24;
const int s_tilesRound = 10;

TileItem::TileItem(QGraphicsWidget* parent, const UrlItem& urlItem, bool editable)
    : QGraphicsRectItem(parent)
    , m_urlItem(urlItem)
    , m_selected(false)
    , m_closeIcon(0)
    , m_editable(editable)
    , m_context(0)
    , m_fixed(false)
    , m_oldRect(-1, -1, -1, -1)
{
#ifndef Q_OS_SYMBIAN
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif    
}

TileItem::~TileItem()
{
    delete m_closeIcon;
}

void TileItem::setTilePos(const QPointF& pos) 
{ 
    setRect(QRectF(pos, rect().size())); 
    update(boundingRect()); 
}

void TileItem::setEditMode(bool on) 
{ 
    if (!m_editable)
        return;
    if (on && !m_closeIcon) {
        m_closeIcon = new QImage(":/data/icon/48x48/close_item_48.png");
        setEditIconRect();
    } else if (!on) {
        delete m_closeIcon;
        m_closeIcon = 0;
    }
    update(boundingRect());
}

void TileItem::setEditIconRect()
{
    if (!m_closeIcon)
        return;
    m_closeIconRect = QRectF(QPointF(rect().width(), 0), QSizeF(s_closeIconSize, s_closeIconSize));
    m_closeIconRect.moveLeft(m_closeIconRect.left() - m_closeIconRect.width()/2 - 5);
    m_closeIconRect.moveTop(m_closeIconRect.top() - m_closeIconRect.height()/2 + 5);
}

void TileItem::paintExtra(QPainter* painter)
{
    QRectF r(rect());
    if (m_closeIcon)
        painter->drawImage(r.topLeft() + m_closeIconRect.topLeft(), *m_closeIcon);

    if (m_selected) {
        painter->setBrush(QColor(80, 80, 80, 160));
        painter->drawRoundedRect(r, 1, 1);
    }
}

void TileItem::addDropShadow(QPainter& painter, const QRectF rect)
{
    // FIXME: dropshadow shouldnt be a rect
    painter.setPen(QColor(40, 40, 40));
    painter.setBrush(QColor(20, 20, 20));
    QRectF r(rect); r.moveTopLeft(r.topLeft() + QPointF(3, 3));
    painter.drawRoundedRect(r, s_tilesRound, s_tilesRound);
}

QRectF TileItem::boundingRect() const
{
    QRectF r(rect());
    r.adjust(0, -s_closeIconSize, s_closeIconSize, 0);
    return r;
}

void TileItem::layoutTile()
{
    if (rect().size() == m_oldRect.size())
        return;
    m_oldRect = rect();
    doLayoutTile();
    setEditIconRect();
}

void TileItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    // expand it to fit thumbs
    QRectF r(m_closeIconRect);
    r.moveTopLeft(r.topLeft() + rect().topLeft());
    r.adjust(-s_closeIconClickAreaMargin, -s_closeIconClickAreaMargin, s_closeIconClickAreaMargin, s_closeIconClickAreaMargin);

    if (m_longpressTime.elapsed() > s_longpressTimeoutThreshold) {
        emit itemEditingMode(this);
    } else if (m_closeIcon && r.contains(event->pos())) {
        // async item selection, give chance to render the item selected/closed.
        QTimer::singleShot(200, this, SLOT(closeItem()));
    } else {    
        m_selected = true;
        update(boundingRect());
        QTimer::singleShot(200, this, SLOT(activateItem()));
    }
}

void TileItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    m_longpressTime.start();        
}

void TileItem::activateItem()
{
    emit itemActivated(this);
}

void TileItem::closeItem()
{
    emit itemClosed(this);
}

////////////////////////////////////////////////////////////////////////////////
ThumbnailTileItem::ThumbnailTileItem(QGraphicsWidget* parent, const UrlItem& urlItem, bool editable)
    : TileItem(parent, urlItem, editable)
{
    if (!urlItem.thumbnail())
        m_defaultIcon = QImage(":/data/icon/48x48/defaulticon_48.png");
}

ThumbnailTileItem::~ThumbnailTileItem()
{
}

void ThumbnailTileItem::doLayoutTile()
{
    const QFont& f = FontFactory::instance()->small();
    QRectF r(QPointF(0, 0), rect().size()); 
    r.adjust(s_tilesRound/2, s_tilesRound/2, -(s_tilesRound/2), 0);

    m_textRect = r;
    m_thumbnailRect = r;

    if (!m_defaultIcon.isNull()) {
        m_thumbnailRect.setSize(m_defaultIcon.rect().size());
        m_thumbnailRect.moveCenter(rect().center() - rect().topLeft());
    } else {
        // stretch thumbnail
        m_thumbnailRect.adjust(0, 0, 0, -(QFontMetrics(f).height() + 3));
    }
    m_title = QFontMetrics(f).elidedText(m_urlItem.title(), Qt::ElideRight, m_textRect.width() - s_tilesRound);
    // scale on the fly, only when default icon is not present
    if (m_defaultIcon.isNull())
        m_scaledThumbnail = m_urlItem.thumbnail()->scaled(m_thumbnailRect.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

void ThumbnailTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    QRectF r(rect()); 
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    // QGraphicsDropShadowEffect doesnt perform well on n900.
    addDropShadow(*painter, r);
 
    painter->setBrush(Qt::white);
    painter->setPen(Qt::gray);
    painter->drawRoundedRect(r, s_tilesRound, s_tilesRound);
    // thumbnail
    if (m_defaultIcon.isNull())
        painter->drawImage(r.topLeft() + m_thumbnailRect.topLeft(), m_scaledThumbnail, m_thumbnailRect);
    else
        painter->drawImage(r.topLeft() + m_thumbnailRect.topLeft(), m_defaultIcon);
    painter->setFont(FontFactory::instance()->small());
    painter->setPen(Qt::black);
    // title
    QRectF textRect(m_textRect);
    textRect.moveTopLeft(r.topLeft() + textRect.topLeft());
    painter->drawText(textRect, Qt::AlignHCenter|Qt::AlignBottom, m_title);
    paintExtra(painter);
}

NewWindowTileItem::NewWindowTileItem(QGraphicsWidget* parent, const UrlItem& item)
    : ThumbnailTileItem(parent, item, false)

{
}

void NewWindowTileItem::activateItem()
{
    // start growing anim
    setZValue(100);
    QPropertyAnimation* moveAnim = new QPropertyAnimation(this, "rect");
    moveAnim->setDuration(500);
    moveAnim->setStartValue(rect());
    // FIXME find out how to make it full view, for now just hack it
    QRectF r(parentWidget()->geometry());
    r.adjust(scene()->sceneRect().right() - r.right(), 0, 0, 0);
    moveAnim->setEndValue(r);

    moveAnim->setEasingCurve(QEasingCurve::OutQuad);
    moveAnim->start();
    connect(moveAnim, SIGNAL(finished()), this, SLOT(newWindowAnimFinished()));
}

void NewWindowTileItem::newWindowAnimFinished()
{
    emit itemActivated(this);
}

void NewWindowTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::white);
    painter->setBrush(m_selected ? Qt::white : Qt::black);
    painter->drawRoundedRect(rect(), s_tilesRound, s_tilesRound);

    painter->setFont(FontFactory::instance()->small());
    painter->setPen(Qt::white);
    painter->drawText(rect(), Qt::AlignCenter, "Open new tab");
}

NewWindowMarkerTileItem::NewWindowMarkerTileItem(QGraphicsWidget* parent, const UrlItem& item)
    : ThumbnailTileItem(parent, item, false)

{
    setFixed(true);
}

void NewWindowMarkerTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    QPen p(Qt::DashLine);
    p.setColor(QColor(100, 100, 100));
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(rect(), s_tilesRound, s_tilesRound);
}

//
ListTileItem::ListTileItem(QGraphicsWidget* parent, const UrlItem& urlItem, bool editable)
    : TileItem(parent, urlItem, editable)
{
}

void ListTileItem::doLayoutTile()
{
    QRectF r(QPointF(0, 0), rect().size()); 
    r.adjust(s_hTextMargin, 0, -s_hTextMargin, 0);
    m_titleRect = r;
    m_urlRect = r;

    const QFont& fmedium = FontFactory::instance()->medium();
    const QFont& fsmall = FontFactory::instance()->small();
    QFontMetrics fmfmedium(fmedium);
    QFontMetrics fmfsmall(fsmall);

    int fontHeightRatio = r.height() / (fmfmedium.height() + fmfsmall.height() + 5);
    m_titleRect.setHeight(fmfmedium.height() * fontHeightRatio);
    m_urlRect.setTop(m_titleRect.bottom() + 5); 

    m_title = fmfmedium.elidedText(m_urlItem.title(), Qt::ElideRight, r.width());
    m_url = fmfsmall.elidedText(m_urlItem.url().toString(), Qt::ElideRight, r.width());
}

void ListTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    layoutTile();

    QRectF r(rect()); 

    // QGraphicsDropShadowEffect doesnt perform well on n900.
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
 
    painter->setBrush(Qt::white);
    painter->setPen(Qt::gray);
    painter->drawRoundedRect(r, 2, 2);

    QRectF textRect(m_titleRect);
    textRect.moveTopLeft(textRect.topLeft() + r.topLeft());
    painter->setPen(Qt::black);
    painter->setFont(FontFactory::instance()->medium());
    painter->drawText(textRect, Qt::AlignLeft|Qt::AlignVCenter, m_title);

    textRect = m_urlRect;
    textRect.moveTopLeft(textRect.topLeft() + r.topLeft());
    painter->setPen(QColor(110, 110, 110));
    painter->setFont(FontFactory::instance()->small());
    painter->drawText(textRect, Qt::AlignLeft|Qt::AlignVCenter, m_url);

    paintExtra(painter);
}


