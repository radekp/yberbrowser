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

class UrlItem : public QObject {
    Q_OBJECT
public:
    UrlItem();
    UrlItem(const QUrl& url, const QString& title, QImage* image = 0);
    ~UrlItem();

    bool thumbnailAvailable() const { return (m_thumbnail != 0 || !m_thumbnailPath.isEmpty()); }
    void setThumbnail(QImage* thumbnail);
    QImage* thumbnail();
    void setThumbnailPath(const QString& path) { m_thumbnailPath = path; }
    QString thumbnailPath();

Q_SIGNALS:
    void thumbnailChanged();

public:
    QUrl m_url;
    QString m_title;
    uint m_refcount;
    uint m_lastAccess;
    bool m_thumbnailChanged;

private:
    QImage* m_thumbnail;
    QString m_thumbnailPath;
};

typedef QList<UrlItem*> UrlList;

#endif

