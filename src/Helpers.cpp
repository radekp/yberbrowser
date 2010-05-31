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

#include "Helpers.h"
#include "FontFactory.h"
#include "Settings.h"

#include <QFileInfo>
#include <QImage>
#include <QPropertyAnimation>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QPen>
#include <QPainter>
#include <QLinearGradient>
#include <QNetworkRequest>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <QDebug>

static int s_maxUrlItems = 50;

class NotificationWidget : public QGraphicsWidget {
    Q_OBJECT
public: 
    static void show(const QString& text, QGraphicsWidget* parent);

private:
    NotificationWidget(const QString& text, QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void doShow();

private Q_SLOTS:
    void startAnimation();
    void animFinished();

private:
    QString m_text;
};

static NotificationWidget* s_notification = 0;

void NotificationWidget::show(const QString& text, QGraphicsWidget* parent)
{
    if (s_notification)
        return;

    s_notification = new NotificationWidget(text, parent);
    s_notification->doShow();
}

NotificationWidget::NotificationWidget(const QString& text, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_text(text)
{
}

void NotificationWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(Qt::black);
    painter->setBrush(QColor(80, 80, 80));

    painter->drawRect(rect());
 
    painter->setFont(FontFactory::instance()->medium());
    painter->setPen(Qt::white);
    painter->drawText(rect(), Qt::AlignHCenter|Qt::AlignVCenter, m_text);
}

void NotificationWidget::doShow()
{
    QRectF r(parentWidget()->rect());
    const QFont& f = FontFactory::instance()->medium();
    int height = QFontMetrics(f).height() + 2;
    
    r.setTop(r.center().y()/2 - height/2);
    r.setHeight(height);
    setGeometry(r);
    QTimer::singleShot(800, this, SLOT(startAnimation()));
}

void NotificationWidget::startAnimation()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "opacity");
    anim->setDuration(400);
    anim->setStartValue(1);
    anim->setEndValue(0);

    anim->setEasingCurve(QEasingCurve::Linear);
    anim->start();
    connect(anim, SIGNAL(finished()), this, SLOT(animFinished()));
}

void NotificationWidget::animFinished()
{
    delete s_notification;
    s_notification = 0;
}

void notification(const QString& text, QGraphicsWidget* parent)
{
    NotificationWidget::show(text, parent);
}

QUrl urlFromUserInput(const QString& string)
{
    QString input(string);
    QFileInfo fi(input);
    if (fi.exists() && fi.isRelative())
        input = fi.absoluteFilePath();

    return QUrl::fromUserInput(input);
}

void internalizeUrlList(UrlList& list, const QString& fileName, uint version)
{
    // read url store
    // version
    // number of items
    // url, refcount, lastaccess
    QFile store(Settings::instance()->privatePath() + fileName);

    if (store.open(QFile::ReadWrite)) {
        QDataStream in(&store);
        uint fileVersion;
        in>>fileVersion;
        if (fileVersion == version) {
            int count;
            in>>count;
            for (int i = 0; i < count; ++i) {
                UrlItem item;
                item.internalize(in);
                list.append(item);
            }
        }
        store.close();
    } 
}

void externalizeUrlList(UrlList& list, const QString& fileName, uint version)
{
    // FIXME clean up old thumbnail items that dont fit s_maxUrlItems now.
    int count = qMin(list.size(), s_maxUrlItems);
    QFile store(Settings::instance()->privatePath() + fileName);
    if (store.open(QFile::WriteOnly | QIODevice::Truncate)) {
        QDataStream out(&store);
        out<<version<<count;
        for (int i = 0; i < count; ++i)
            list[i].externalize(out);
        store.close();
    } 
}

#include "Helpers.moc"
