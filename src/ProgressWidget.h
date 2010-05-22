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

#ifndef ProgressWidget_h_
#define ProgressWidget_h_

#include <QObject>
#include <QGraphicsRectItem>
#include <QLinearGradient>

class QWebView;
class ProgressItem;
class QPropertyAnimation;

class ProgressWidget : public QGraphicsRectItem {
public:
    ProgressWidget(QGraphicsItem* parent);
    ~ProgressWidget();

    void updateGeometry(const QRectF& rect);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

public Q_SLOTS:
    void loadStarted();
    void progressChanged(int percentage);
    void loadFinished(bool success);
    void slideFinished();

private:
    void paintBackground(QPainter* painter);
    void paintItems(QPainter* painter, const QRectF& rect, qreal opacity);
    void destroyProgressItems();

    QString m_label;
    QList<ProgressItem*> m_progressItemList;
    int m_lastPercentage;
    QRectF m_progressBoxRect;
    QLinearGradient m_bckgGradient;
    QLinearGradient m_progressGradient;
};

#endif
