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

#ifndef UrlItem_h_
#define UrlItem_h_

#include <QObject>
#include <QUrl>
#include <QString>
#include <QList>

class QImage;

class UrlItem {
public:
    UrlItem();
    UrlItem(const QUrl& url, const QString& title, QImage* thumbnail);
    UrlItem(const UrlItem& item);
    ~UrlItem();

    UrlItem& operator=(const UrlItem& other);
    bool operator<(const UrlItem& other) const;
    bool operator==(const UrlItem& other) const;
    
    QUrl url() const { return m_url; }
    QString title() const { return m_title; }
    uint refcount() const { return m_refcount; }
    uint lastAccess() const { return m_lastAccess; }
    QImage* thumbnail() const { return m_thumbnail; }

    void setRefcount(uint refcount) { m_refcount = refcount; }
    void setLastAccess(uint accessTime) { m_lastAccess = accessTime; }
    void setThumbnail(QImage* thumbnail);

    void externalize(QDataStream& out);
    void internalize(QDataStream& in);

private:
    QUrl m_url;
    QString m_title;
    uint m_refcount;
    uint m_lastAccess;
    QImage* m_thumbnail;
    QString m_thumbnailPath;
    bool m_thumbnailChanged;
};

typedef QList<UrlItem> UrlList;

#endif

