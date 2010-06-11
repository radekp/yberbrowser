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

#include "ProgressWidget.h"

#include <QPropertyAnimation>
#include <QPainter>
#include <QDebug>

static QString s_initialProgressText("Loading...");
static qreal s_progressbarOpacity = 0.8;

class ProgressItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ProgressItem(const QRectF& rect, ProgressWidget& parent) { m_rect = rect; m_parent = &parent; }

    void setOpacity(qreal opacity) { m_opacity = opacity; m_parent->update();}
    qreal opacity() const { return m_opacity; }
    void setRect(const QRectF& rect) { m_rect = rect; }
    QRectF rect() const { return m_rect; }

private:
    ProgressWidget* m_parent;
    qreal m_opacity;
    QRectF m_rect;
};

ProgressWidget::ProgressWidget(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , m_lastPercentage(0)
{
    hide();
    // setup gradients
    // background
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(120, 120, 120)) << QGradientStop(0.30, QColor(209, 204, 234)) << QGradientStop(0.50, QColor(219, 214, 244)) << QGradientStop(0.70, QColor(209, 204, 234)) << QGradientStop(1.00, QColor(120, 120, 120));
    for (int j=0; j<stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);

    // progress items
    QGradientStops pstops;
    pstops << QGradientStop(0.00, QColor(226, 255, 51)) << QGradientStop(0.50, QColor(39, 115 , 3)) << QGradientStop(0.60, QColor(39, 115 , 3)) << QGradientStop(1.00, QColor(226, 255, 51));
    for (int j=0; j<pstops.size(); ++j)
        m_progressGradient.setColorAt(pstops.at(j).first, pstops.at(j).second);
}

ProgressWidget::~ProgressWidget()
{
    destroyProgressItems();
}

void ProgressWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    paintBackground(painter);
    // paint percentage items
    int i = 0;
    // paint finished items in one go. should merge them, really
    if (m_progressItemList.size() > 1) {
        QRectF rect = m_progressItemList.at(0)->rect();
        for (;i < m_progressItemList.size(); ++i) {
            if (m_progressItemList.at(i)->opacity() != s_progressbarOpacity)
                break;
            rect.setRight(m_progressItemList.at(i)->rect().right());
        }
        if (i > 0)
            paintItems(painter, rect, s_progressbarOpacity);
    }

    for (int j = i; j < m_progressItemList.size(); ++j) 
        paintItems(painter, m_progressItemList.at(j)->rect(), m_progressItemList.at(j)->opacity());

    // paint percentage
    if (m_label.size()) {
        painter->setOpacity(1.0);
        painter->setPen(QColor(40, 40, 40));
        painter->drawText(m_progressBoxRect, Qt::AlignCenter, m_label);
    }
}

void ProgressWidget::paintBackground(QPainter* painter)
{
    painter->setBrush(m_bckgGradient);
    painter->setPen(Qt::black);
    painter->setOpacity(0.8);
    painter->drawRoundedRect(m_progressBoxRect, 3, 1);
}

void ProgressWidget::paintItems(QPainter* painter, const QRectF& rect, qreal opacity)
{
    painter->setBrush(m_progressGradient);
    painter->setPen(Qt::NoPen);
    painter->setOpacity(opacity);
    painter->drawRect(rect);
}
   
void ProgressWidget::loadStarted()
{
    m_label = s_initialProgressText;
    m_lastPercentage = 0;
    show();
}

void ProgressWidget::progressChanged(int percentage)
{
    // todo: find out this magic 10% thing
    if (percentage == m_lastPercentage || percentage <= 10)
        return;
    // no percentage text
    m_label = QString();
    m_lastPercentage = percentage;
    
    // create the new progress item
    QRectF rect(m_progressBoxRect);
    // shrink it
    rect.setY(rect.y() + 1);
    rect.setBottom(rect.bottom() - 1);
    int startX = rect.x() + 1;

    if (m_progressItemList.size())
        startX = m_progressItemList.at(m_progressItemList.size() - 1)->rect().right();

    rect.setRight(rect.x() + (rect.width() * percentage) / 100);
    rect.setLeft(startX);

    // animation
    ProgressItem* item = new ProgressItem(rect, *this);
    m_progressItemList.append(item);

    // assign opacity animation
    QPropertyAnimation* animation = new QPropertyAnimation(item, "opacity");
    animation->setDuration(800);

    animation->setStartValue(0.0);
    animation->setEndValue(s_progressbarOpacity);    

    animation->setEasingCurve(QEasingCurve::InCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ProgressWidget::loadFinished(bool /*success*/)
{
    m_lastPercentage = 0;
}

void ProgressWidget::updateGeometry(const QRectF& rect)
{
    const QFont f = QFont();
    QFontMetrics fm(f);
    int height = fm.height();
    // set the progressbox size to 1/4 of the view
    int width = rect.width();
    // todo: progress width stays the same while loading. make it dynamic 
    if (m_lastPercentage == 0)
        width = qMax(fm.size(Qt::TextSingleLine, s_initialProgressText).width() + 30, int(rect.width() / 4));
    m_progressBoxRect = QRectF(0, rect.bottomLeft().y() - (height + 3), width, height + 3);

    // reset progress items
    for (int i = 0; i < m_progressItemList.size(); ++i) {
        QRectF r = m_progressItemList.at(i)->rect();
        r.setY(m_progressBoxRect.y() + 1);
        r.setBottom(m_progressBoxRect.bottom() - 1);
        m_progressItemList.at(i)->setRect(r);
    }
    // update gradient stops
    m_progressGradient.setStart(m_progressBoxRect.topLeft());
    m_progressGradient.setFinalStop(m_progressBoxRect.bottomLeft());

    m_bckgGradient.setStart(m_progressBoxRect.topLeft());
    m_bckgGradient.setFinalStop(m_progressBoxRect.bottomLeft());
    
    setRect(m_progressBoxRect);
}

void ProgressWidget::destroyProgressItems() 
{
    for (int i = m_progressItemList.size() - 1; i >= 0 ; i--)
        delete m_progressItemList.takeAt(i);
}

#include "ProgressWidget.moc"
