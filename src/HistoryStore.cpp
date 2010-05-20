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

#include "HistoryStore.h"
#include <QDateTime>
#include <QImage>
#include <QTimer>
#include <QRegExp>
#include "Helpers.h"

//#define ENABLE_HISTORYSTORE_DEBUG 1

#if defined(ENABLE_HISTORYSTORE_DEBUG)
#include <QDebug>
#endif

static uint s_currentVersion = 3;

HistoryStore* HistoryStore::instance()
{
    static HistoryStore* historyStore = 0;
    if (!historyStore)
        historyStore = new HistoryStore();
    return historyStore;
}    

HistoryStore::HistoryStore()
    : m_needsPersisting(false)
{
    internalizeUrlList(m_list, "historystore.txt", s_currentVersion);
    if (!m_list.size()) {
#if defined(ENABLE_HISTORYSTORE_DEBUG)
        qDebug() << "HistoryStore: no url store, use default values";
#endif
        // init historystore with some popular urls. prefer non-www for to save space
        m_list.append(UrlItem(QUrl("http://cnn.com/"), "CNN.com - Breaking News, U.S., World, Weather, Entertainment &amp; Video News", 0));
        m_list.append(UrlItem(QUrl("http://news.bbc.co.uk/"), "BBC NEWS | News Front Page", 0));
        m_list.append(UrlItem(QUrl("http://news.google.com/"), "Google News", 0));
        m_list.append(UrlItem(QUrl("http://nokia.com/"), "Nokia - Nokia on the Web", 0));
        m_list.append(UrlItem(QUrl("http://qt.nokia.com/"), "Qt - A cross-platform application and UI framework", 0));
        m_list.append(UrlItem(QUrl("http://ovi.com/"), "Ovi by Nokia", 0));
        m_list.append(UrlItem(QUrl("http://nytimes.com/"), "The New York Times - Breaking News, World News Multimedia", 0));
        m_list.append(UrlItem(QUrl("http://google.com/"), "Google", 0));
    }
}

// FIXME: this is a singleton, dont get properly deleted
HistoryStore::~HistoryStore()
{
    externalize();
}

void HistoryStore::externalize()
{
    if (!m_needsPersisting)
        return;
    externalizeUrlList(m_list, "historystore.txt", s_currentVersion);
    m_needsPersisting = false;
}

void HistoryStore::accessed(const QUrl& url, const QString& title, QImage* thumbnail)
{
    QString accessedHostAndPath = url.host() + url.path();
    int found = -1;
#if defined(ENABLE_HISTORYSTORE_DEBUG)
    qDebug() << "HistoryStore:" << __FUNCTION__ << accessedHost;
#endif
    for (int i = 0; i < m_list.size(); ++i) {
        UrlItem& item = m_list[i];
        if (matchUrls(item.url().host() + item.url().path(), accessedHostAndPath)) {
            item.setRefcount(item.refcount() + 1);
            item.setLastAccess(QDateTime::currentDateTime().toTime_t());
            // move it up if needed
            int j = i;
            // '<=' is for the last access sorting, recently used items move up
            while (--j >= 0 && item.refcount() >= m_list.at(j).refcount()) {}
            // position adjusting and check whether we really moved
            if (++j != i) 
                m_list.move(i, j);
            found = j;
            break;
        }
    }
    
    if (found == -1) {
        // insert to the top of the 1 refcount items. recently used sort
        int i = m_list.size();
        while (--i >= 0 && m_list[i].refcount() == 1) {}
        m_list.insert(++i, UrlItem(url, title, thumbnail));
    } else if (thumbnail) {
        // add thumbnail if not there yet
        m_list[found].setThumbnail(thumbnail);
    }
#if defined(ENABLE_HISTORYSTORE_DEBUG)
    for (int i = 0; i < m_list.size(); ++i)
        qDebug()<<m_list[i].m_url.toString()<<" "<<m_list[i].m_refcount;
#endif
    externalizeSoon();
}

bool HistoryStore::contains(const QString& url)
{
    for (int i = 0; i < m_list.size(); ++i)
        if (m_list[i].url().toString() == url)
            return true;
    return false;
}

QString HistoryStore::match(const QString& url)
{
    if (url.isEmpty())
        return QString();
    for (int i = 0; i < m_list.size(); ++i) {
        // do a very simply startWith matching first.
        QString host = m_list[i].url().host();
        if (host.startsWith(url))
            return host;
        else if (host.startsWith("www.")) {
            host = host.mid(4);
            if (host.startsWith(url))
                return host;
        }
    }
    return QString();
}

void HistoryStore::match(const QString& url, UrlList& matchedItems)
{
    // FIXME should do some crazy regexp, for now just be stupid
    if (url.isEmpty())
        return;
    QString text = url;
    text.replace(" ", "|");
    QRegExp rx(text);
    for (int i = 0; i < m_list.size(); ++i) {
        const UrlItem& item = m_list[i];
        if (item.url().toString().indexOf(rx) > -1 || item.title().indexOf(rx) > -1)
           matchedItems.append(m_list.at(i));
    }
}

void HistoryStore::remove(const QUrl& url)
{
    for (int i = 0; i < m_list.size(); ++i) {
        if (m_list[i].url() == url) {
            m_list.removeAt(i);
            externalizeSoon();
            break;
        }
    }
}

bool HistoryStore::matchUrls(const QString& url1, const QString& url2) 
{
    // www.cnn.com == www.cnn.com
    if (url1 == url2)
        return true;
    // www.cnn.com == cnn.com
    if ((url1.startsWith("www.") && url1.mid(4) == url2) || 
        (url2.startsWith("www.") && url2.mid(4) == url1))
        return true;
    return false;
}

void HistoryStore::externalizeSoon() 
{
    m_needsPersisting = true;
    QTimer::singleShot(2000, this, SLOT(externalize()));
}

