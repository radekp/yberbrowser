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

#ifndef AutoScrollTest_h
#define AutoScrollTest_h

#include "yberconfig.h"
#include <QGraphicsWidget>
#include <QTime>
#include <QTimer>
#include <QPoint>

class QGraphicsSceneMouseEvent;
class PannableViewport;
class WebView;

class AutoScrollTest : public QGraphicsWidget
{
    Q_OBJECT
public:
    AutoScrollTest(PannableViewport* viewport, WebView* webView, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~AutoScrollTest();

    void starScrollTest();

public Q_SLOTS:
    void doScroll();
    void fpsTick();
    void loadFinished(bool);
    void scrollTimeout();

Q_SIGNALS:
    void finished();

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    void addAreaDividerItem(int);
    void addHorizontalLineItem(int);
    void addTextItem(const QRectF&, const QString&);
    void addTransparentRectangleItem(const QRectF&);
    void displayResult();
    qreal getYValue(qreal fps);

private:
    PannableViewport* m_viewport;
    WebView* m_webView;
    QTimer m_scrollTimer;
    unsigned int m_scrollIndex;
    int m_scrollValue;
    QTime m_fpsTimestamp;
    QTimer m_fpsTimer;
    unsigned int m_fpsTicks;
    QList<int> m_fpsValues;
    int m_min;
    int m_max;
    int m_avg;
};
#endif
