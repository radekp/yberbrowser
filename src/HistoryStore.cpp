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
    internalizeUrlList(m_list, "historystore.txt");
    if (!m_list.size()) {
#if defined(ENABLE_HISTORYSTORE_DEBUG)
        qDebug() << "HistoryStore: no url store, use default values";
#endif
        // init historystore with some popular urls. prefer non-www for to save space
        m_list.append(new UrlItem(QUrl("http://cnn.com/"), "CNN.com - Breaking News, U.S., World, Weather, Entertainment &amp; Video News"));
        m_list.append(new UrlItem(QUrl("http://news.bbc.co.uk/"), "BBC NEWS | News Front Page"));
        m_list.append(new UrlItem(QUrl("http://news.google.com/"), "Google News"));
        m_list.append(new UrlItem(QUrl("http://nokia.com/"), "Nokia - Nokia on the Web"));
        m_list.append(new UrlItem(QUrl("http://qt.nokia.com/"), "Qt - A cross-platform application and UI framework"));
        m_list.append(new UrlItem(QUrl("http://ovi.com/"), "Ovi by Nokia"));
        m_list.append(new UrlItem(QUrl("http://nytimes.com/"), "The New York Times - Breaking News, World News Multimedia"));
        m_list.append(new UrlItem(QUrl("http://google.com/"), "Google"));
    }
}

// FIXME: this is a singleton, dont get properly deleted
HistoryStore::~HistoryStore()
{
    externalize();
    for (int i = 0; i < m_list.size(); ++i)
        delete m_list.takeAt(i);
}

void HistoryStore::externalize()
{
    if (!m_needsPersisting)
        return;
    externalizeUrlList(m_list, "historystore.txt");
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
        UrlItem* item = m_list[i];
        if (matchUrls(item->m_url.host() + item->m_url.path(), accessedHostAndPath)) {
            item->m_refcount++;
            item->m_lastAccess = QDateTime::currentDateTime().toTime_t();
            // move it up if needed
            int j = i;
            // '<=' is for the last access sorting, recently used items move up
            while (--j >= 0 && item->m_refcount >= m_list.at(j)->m_refcount) {}
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
        while (--i >= 0 && m_list.at(i)->m_refcount == 1) {}
        m_list.insert(++i, new UrlItem(url, title, thumbnail));
    } else if (thumbnail) {
        // add thumbnail if not there yet
        m_list[found]->setThumbnail(thumbnail);
    }
#if defined(ENABLE_HISTORYSTORE_DEBUG)
    for (int i = 0; i < m_list.size(); ++i)
        qDebug()<<m_list.at(i)->m_url.toString()<<" "<<m_list.at(i)->m_refcount;
#endif
    externalizeSoon();
}

UrlItem* HistoryStore::get(const QString& url)
{
    for (int i = 0; i < m_list.size(); ++i)
        if (m_list.at(i)->m_url.toString() == url)
            return m_list.at(i);
    return 0;
}

bool HistoryStore::contains(const QString& url)
{
    return get(url) != 0;
}

QString HistoryStore::match(const QString& url)
{
    if (url.isEmpty())
        return QString();
    for (int i = 0; i < m_list.size(); ++i) {
        // do a very simply startWith matching first.
        QString host = m_list.at(i)->m_url.host();
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
        UrlItem* item = m_list.at(i);
        if (item->m_url.toString().indexOf(rx) > -1 || item->m_title.indexOf(rx) > -1)
           matchedItems.append(m_list.at(i));
    }
}

void HistoryStore::remove(const QString& url)
{
    for (int i = 0; i < m_list.size(); ++i) {
        if (m_list.at(i)->m_url.toString() == url) {
            delete m_list.takeAt(i);
            externalizeSoon();
            break;
        }
    }
}

void HistoryStore::remove(UrlItem& item)
{
    for (int i = 0; i < m_list.size(); ++i) {
        if (m_list.at(i) == &item) {
            delete m_list.takeAt(i);
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

