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
    , m_thumbnailChanged(false)
    , m_thumbnail(0)
{
}

UrlItem::UrlItem(const QUrl& url, const QString& title, QImage* thumbnail)
    : m_url(url)
    , m_title(title)
    , m_refcount(1)
    , m_lastAccess(QDateTime::currentDateTime().toTime_t())
    , m_thumbnailChanged(thumbnail)
    , m_thumbnail(thumbnail)
{
}

UrlItem::~UrlItem()
{
    delete m_thumbnail;
}

void UrlItem::setThumbnail(QImage* thumbnail) 
{ 
    m_thumbnail = thumbnail; 
    m_thumbnailChanged = true;
    emit thumbnailChanged();
}

QImage* UrlItem::thumbnail()
{
    // load thumbnails on demand
    if (m_thumbnail)
        return m_thumbnail;
    // available?
    if (!m_thumbnailPath.size())
        return 0;
    m_thumbnail = new QImage(Settings::instance()->privatePath() + m_thumbnailPath);
    return m_thumbnail;
}

QString UrlItem::thumbnailPath()
{
    if (!m_thumbnail)
        return QString();
    if (!m_thumbnailPath.size())
        m_thumbnailPath = QString::number(m_lastAccess + rand()) + ".png";
    return m_thumbnailPath;
}

