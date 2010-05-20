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

#include "BookmarkStore.h"
#include <QImage>
#include <QPixmap>
#include <QTimer>

#include "Helpers.h"

#include <QDebug>

static uint s_currentVersion = 3;

BookmarkStore* BookmarkStore::instance()
{
    static BookmarkStore* bookmarkStore = 0;
    if (!bookmarkStore)
        bookmarkStore = new BookmarkStore();
    return bookmarkStore;
}    

BookmarkStore::BookmarkStore()
    : m_needsPersisting(false)
{
    internalizeUrlList(m_list, "bookmarkstore.txt", s_currentVersion);
    if (!m_list.size()) {
        // FIXME move icons out of the res file. 
        add(QUrl("http://www.facebook.com/"), "Welcome to facebook");
        add(QUrl("http://www.google.com/"), "Google");
        add(QUrl("http://www.msn.com/"), "MSN.com");
        add(QUrl("http://twitter.com/"), "Twitter");
        add(QUrl("http://www.wikipedia.org/"), "Wikipedia");
        add(QUrl("http://www.yahoo.com/"), "Yahoo!");
    }
}

// FIXME: this is a singleton, dont get properly deleted
BookmarkStore::~BookmarkStore()
{
    externalize();
}

void BookmarkStore::add(const QUrl& url, const QString& title)
{
#if 0
    // FIXME webkit provides empty icons
    QImage* image = new QImage(favicon.pixmap(QSize(16,16)).toImage());;
    if (image->size() == QSize(0, 0)) {
        delete image;
        image = 0;
    }
#endif
    UrlItem newItem(url, title, 0);
    m_list.insert(qUpperBound(m_list.begin(), m_list.end(), newItem), newItem);

    externalizeSoon();
}

void BookmarkStore::remove(const QUrl& url)
{
    for (int i = 0; i < m_list.size(); ++i) {
        if (m_list[i].url() == url) {
            m_list.removeAt(i);
            externalizeSoon();
            break;
        }
    }
}

void BookmarkStore::externalizeSoon()
{
    m_needsPersisting = true;
    QTimer::singleShot(5000, this, SLOT(externalize()));
}

void BookmarkStore::externalize()
{
    if (!m_needsPersisting)
        return;
    externalizeUrlList(m_list, "bookmarkstore.txt", s_currentVersion);
    m_needsPersisting = false;
}

