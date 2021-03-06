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

#ifndef WebView_h
#define WebView_h

#if USE_WEBKIT2
#define PLATFORM(x) 0
#include <stdint.h>
#include <WebKit2/qgraphicswkview.h>
#else
#include <qgraphicswebview.h>
#endif
#include "yberconfig.h"
#include "PannableViewport.h"

class WebView : public
#if USE_WEBKIT2
    QGraphicsWKView
#else
    QGraphicsWebView
#endif
    {
    Q_OBJECT
public:
#if USE_WEBKIT2
    WebView(WKPageNamespaceRef namespaceRef, QGraphicsItem* parent = 0);
#else
    WebView(QGraphicsItem* parent = 0);
#endif
    void paint(QPainter* p, const QStyleOptionGraphicsItem* i, QWidget* w= 0);
    unsigned int fpsTicks() const { return m_fpsTicks; }

private:
    Q_DISABLE_COPY(WebView)
    void applyPageSettings();

private:
    unsigned int m_fpsTicks;
};

#endif
