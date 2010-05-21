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

#include "UrlItem.h"
#include <QDateTime>
#include <QImage>
#include <QDebug>
#include "Settings.h"

UrlItem::UrlItem()
    : m_refcount(0)
    , m_lastAccess(0)
    , m_thumbnail(0)
    , m_thumbnailChanged(false)
{
}

UrlItem::UrlItem(const QUrl& url, const QString& title, QImage* thumbnail)
    : m_url(url)
    , m_title(title)
    , m_refcount(1)
    , m_lastAccess(QDateTime::currentDateTime().toTime_t())
    , m_thumbnail(thumbnail)
    , m_thumbnailChanged(true)
{
}

UrlItem::UrlItem(const UrlItem& item)
    : m_url(item.m_url)
    , m_title(item.m_title)
    , m_refcount(item.m_refcount)
    , m_lastAccess(item.m_lastAccess)
    , m_thumbnail(item.m_thumbnail ? new QImage(*item.m_thumbnail) : 0)
    , m_thumbnailPath(item.m_thumbnailPath)
    , m_thumbnailChanged(item.m_thumbnailChanged)
{
}

UrlItem::~UrlItem()
{
    delete m_thumbnail;
}

UrlItem& UrlItem::operator=(const UrlItem& other)
{
    m_url = other.m_url;
    m_title = other.m_title;
    m_refcount = other.m_refcount;
    m_lastAccess = other.m_lastAccess;
    m_thumbnailChanged = other.m_thumbnailChanged;
    m_thumbnail = new QImage(*other.m_thumbnail);
    m_thumbnailPath = other.m_thumbnailPath;
    return *this;
}

bool UrlItem::operator==(const UrlItem& other) const
{
    return m_url == other.m_url;
}

bool UrlItem::operator<(const UrlItem& other) const
{
    return m_title.toLower() < other.m_title.toLower();
}

void UrlItem::setThumbnail(QImage* thumbnail) 
{ 
    delete m_thumbnail;

    m_thumbnail = thumbnail; 
    m_thumbnailChanged = true;
}

void UrlItem::externalize(QDataStream& out)
{
    if (m_thumbnail && m_thumbnailChanged) {
        if (m_thumbnailPath.isEmpty())
            m_thumbnailPath = QString::number(m_lastAccess + rand()) + ".png";
        m_thumbnail->save(Settings::instance()->privatePath() + m_thumbnailPath);
    }
    m_thumbnailChanged = false;

    out<<m_url.toString()<<m_title<<m_refcount<<m_lastAccess<<m_thumbnailPath;
}

void UrlItem::internalize(QDataStream& in)
{
    QString urlStr; 
    in>>urlStr>>m_title>>m_refcount>>m_lastAccess>>m_thumbnailPath;
    m_url = urlStr;
    if (!m_thumbnailPath.isEmpty())
        m_thumbnail = new QImage(Settings::instance()->privatePath() + m_thumbnailPath);
}

