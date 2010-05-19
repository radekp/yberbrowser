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

#include "WebView.h"
#if defined(ENABLE_PAINT_DEBUG)
#include <QTime>
#include <QStyleOptionGraphicsItem>
#endif
WebView::WebView(QGraphicsItem* parent)
    : QGraphicsWebView(parent)
    , m_fpsTicks(0)
{
}

void WebView::paint(QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* w)
{
    m_fpsTicks++;
#if defined(ENABLE_PAINT_DEBUG)
    QTime t;
    t.start();
#endif
    QGraphicsWebView::paint(p, option, w);
#if defined(ENABLE_PAINT_DEBUG)
    qDebug() << __FUNCTION__ << "ticks:" << m_fpsTicks << t.elapsed() << option->exposedRect.toAlignedRect();
#endif
}
