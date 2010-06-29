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

#include "AutoScrollTest.h"
#include "BrowsingView.h"
#include "FontFactory.h"
#include "PannableViewport.h"
#include "WebViewport.h"
#include "WebViewportItem.h"
#include "WebView.h"

#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QFontMetrics>

const int s_scrollPixels[] = {10, 20, 30, 40, 50, 10};
const int s_scrollPixelsTimeot[] = {3000, 3000, 4000, 4000, 3000, 2000};
const int s_xAxisInFPS[] = {10, 20, 60};
const QColor s_xFPSValueColor(10, 255, 10);
const QColor s_FPSTextColor(10, 255, 10);
const QColor s_FPSDividerColor(255, 10, 10);
const QColor s_FPSLineColor(10, 255, 10);
const QColor s_bckColor(20, 20, 20);
const qreal s_bckTransparency = 0.9;
const int s_fpsCheckTimeout = 100;
const int s_scrollTimeout = 10;

AutoScrollTest::AutoScrollTest(PannableViewport* viewport, WebView* webView, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_viewport(viewport)
    , m_webView(webView)
    , m_scrollTimer(this)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);

    connect(&m_scrollTimer, SIGNAL(timeout()), this, SLOT(doScroll()));
    connect(&m_fpsTimer, SIGNAL(timeout()), this, SLOT(fpsTick()));
}

AutoScrollTest::~AutoScrollTest()
{
}

void AutoScrollTest::starScrollTest()
{
    // load news.google.com if nothing is loaded.
    if (m_webView->url().isEmpty()) {
        m_webView->load(QUrl("http://news.google.com"));
        connect(m_webView->page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        return;
    }

    m_scrollIndex = 0;
    m_fpsTicks = m_webView->fpsTicks();
    m_fpsTimestamp.start();
    m_fpsTimer.start(s_fpsCheckTimeout);
    m_scrollTimer.start(s_scrollTimeout);   
    m_scrollValue = s_scrollPixels[0];
    QTimer::singleShot(s_scrollPixelsTimeot[0], this, SLOT(scrollTimeout()));
}

void AutoScrollTest::doScroll()
{
    QPointF panPos(m_viewport->panPos().x(), m_viewport->panPos().y() - m_scrollValue);
    m_viewport->setPanPos(panPos);

    // switch direction
    if (panPos.y() != m_viewport->panPos().y())
        m_scrollValue = -m_scrollValue;
}

void AutoScrollTest::fpsTick()
{
    int prevfps = m_fpsTicks;
    m_fpsTicks = m_webView->fpsTicks();

    double dt = m_fpsTimestamp.restart();
    double dticks = m_fpsTicks - prevfps;
    if (dt)
        m_fpsValues.append((dticks *  1000.) / dt);
}

void AutoScrollTest::loadFinished(bool success)
{
    // dont start scrolling right after page is loaded. it alters the result
    if (success)
        QTimer::singleShot(1000, this, SLOT(starScrollTest()));
}

void AutoScrollTest::scrollTimeout()
{
    m_fpsValues.append(-1);
    // finished?
    if (++m_scrollIndex < sizeof(s_scrollPixelsTimeot) / sizeof(int)) {
        m_scrollValue = s_scrollPixels[m_scrollIndex < sizeof(s_scrollPixels) / sizeof(int) ? m_scrollIndex : 0];
        QTimer::singleShot(s_scrollPixelsTimeot[m_scrollIndex], this, SLOT(scrollTimeout()));
    } else {
        m_scrollTimer.stop();
        m_fpsTimer.stop();
        displayResult();
    }
}

qreal AutoScrollTest::getYValue(qreal fps) 
{
    QRectF r(rect());
    // squeeze it to the middle 
    qreal ed = r.height() / 2 / (m_max - m_min);
    return (r.bottom() - (r.height() / 4) - (ed * (fps - m_min)));
}

void AutoScrollTest::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    emit finished();
}

void AutoScrollTest::addAreaDividerItem(int x)
{
    QRectF r(rect());
    QGraphicsLineItem* areaDivider = new QGraphicsLineItem(x, r.top(), x, r.bottom(), this);
    areaDivider->setPen(QPen(QBrush(s_FPSDividerColor), 1, Qt::DashLine));
}

void AutoScrollTest::addHorizontalLineItem(int y)
{
    QRectF r(rect());
    QGraphicsLineItem* lineItem = new QGraphicsLineItem(r.left(), y, r.right(), y, this);
    lineItem->setPen(QPen(QBrush(s_xFPSValueColor), 1, Qt::DotLine));
}

void AutoScrollTest::addTextItem(const QRectF& rect, const QString& text)
{
    const QFont& f = FontFactory::instance()->small();
    QFontMetrics m(f);

    QGraphicsSimpleTextItem* fpsTextItem = new QGraphicsSimpleTextItem(text, this);
    fpsTextItem->setFont(f);
    fpsTextItem->setPos(rect.left() + rect.width()/2 - m.width(text)/2, rect.top() + rect.height() / 2 - m.height() / 2);
    fpsTextItem->setPen(s_FPSTextColor);
    fpsTextItem->setBrush(s_FPSTextColor);
}

void AutoScrollTest::addTransparentRectangleItem(const QRectF& rect)
{
    QGraphicsRectItem* bckg = new QGraphicsRectItem(rect, this);
    bckg->setBrush(s_bckColor);
    bckg->setOpacity(s_bckTransparency);
}

void AutoScrollTest::displayResult()
{
    if (!m_fpsValues.size())
        return;
    
    // background
    QRectF r(rect());

    int xPadding = r.width() / 10;
    int yPadding = r.height() / 10;
    r.adjust(xPadding, yPadding, -xPadding, -yPadding);
    addTransparentRectangleItem(r);

    // calculate min max and avg first to setup the grid
    m_min = m_fpsValues.at(0);
    m_max = m_min;
    m_avg = m_fpsValues.at(0);
    int count = 1;
    for (int i = 1; i < m_fpsValues.size(); ++i) {
        // reserved value
        if (m_fpsValues.at(i) == -1)
            continue;
        m_max = qMax(m_fpsValues.at(i), m_max);
        m_min = qMin(m_fpsValues.at(i), m_min);
        m_avg+=m_fpsValues.at(i);
        count++;
    }
    m_avg/=count++;

    // create path for fps
    QPainterPath path;

    qreal ed = r.width() / m_fpsValues.size();
    qreal x = r.left(); 
    qreal y = getYValue(m_fpsValues.at(0));
    path.moveTo(x, y);
    // add fps items and scroll divider
    for (int i = 1; i < m_fpsValues.size(); ++i) {
        x = r.left() + i*ed;

        // -1 indicates new scroll sections (up, down, up, down)
        if (m_fpsValues.at(i) == -1) {
            addAreaDividerItem(x);
            continue;
        }
        // add fps value to the path
        y = getYValue(m_fpsValues.at(i));
        path.lineTo(x, y);
    }
    
    QGraphicsPathItem* results = new QGraphicsPathItem(path, this);
    results->setPen(QPen(QBrush(s_FPSLineColor), 1));

    // left side rectangle for fps text
    QRectF sideRect(r); sideRect.moveLeft(sideRect.left() - 30); sideRect.setWidth(20);
    addTransparentRectangleItem(sideRect);
    for (unsigned int i = 0; i < sizeof(s_xAxisInFPS) / sizeof(int); ++i) {
        y = getYValue(s_xAxisInFPS[i]);
        // out of bounds? could happen on non target env, when fps average is so high
        if (!r.contains(r.left(), y))
            continue;
        addHorizontalLineItem(y);
        addTextItem(QRectF(sideRect.left(), y - 10, sideRect.width(), 20), QString::number(s_xAxisInFPS[i]));
    }    

    // add avg as an fps horizontal line too
    addHorizontalLineItem(getYValue(m_avg));
    addTextItem(QRectF(sideRect.left(), getYValue(m_avg) - 10, sideRect.width(), 20), QString::number(m_avg));

    // top rectangle item with min, max and avg
    QRectF topRect(r); topRect.moveTop(sideRect.top() - 30); topRect.setHeight(20);
    addTransparentRectangleItem(topRect);
    addTextItem(QRect(topRect.left() + 5, topRect.top(), 50, topRect.height()), QString("Min:" + QString::number(m_min) + "fps"));
    addTextItem(QRect(topRect.left() + 100, topRect.top(), 50, topRect.height()), QString("Max:" + QString::number(m_max) + "fps"));
    addTextItem(QRect(topRect.left() + 200, topRect.top(), 50, topRect.height()), QString("Avg:" + QString::number(m_avg) + "fps"));
}

