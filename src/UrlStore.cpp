#include "UrlStore.h"
#include <QDataStream>
#include <QFile>
#include <QDateTime>
#include <iostream>
#include <QImage>
#include <QTimer>

//#define ENABLE_URLSTORE_DEBUG 1

#if defined(ENABLE_URLSTORE_DEBUG)
#include <QDebug>
#endif

// FIXME: remove privPath

static uint currentVersion = 2;
QString UrlStore::s_thumbnailDir = QString();

UrlStore::UrlStore()
    : m_needsPersisting(false)
{
    internalize();
}

UrlStore::~UrlStore()
{
    externalize();
    for (int i = 0; i < m_list.size(); ++i)
        delete m_list.takeAt(i);
}

void UrlStore::internalize()
{
    // read url store
    // version
    // number of items
    // url, refcount, lastaccess
#if defined(ENABLE_URLSTORE_DEBUG)
    qDebug() << "UrlStore:" << __FUNCTION__ << "urlstore.txt"<<endl;
#endif
    QFile store(thumbnailDir() + "urlstore.txt");

    if (store.open(QFile::ReadWrite)) {
        QDataStream in(&store);
        uint version;
        in>>version;
        if (version == currentVersion) {
            int count;
            in>>count;
            for (int i = 0; i < count; ++i) {
                UrlItem* item = new UrlItem();
                QString url;
                in>>url; item->m_url = url;
                in>>item->m_title>>item->m_refcount>>item->m_lastAccess>>item->m_thumbnailPath;
                m_list.append(item);
            }
        }
        store.close();
    } 

    if (!m_list.size()) {
#if defined(ENABLE_URLSTORE_DEBUG)
        qDebug() << "UrlStore: no url store, use default values";
#endif
        // init urlstore with some popular urls. prefer non-www for to save space
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

void UrlStore::externalize()
{
    if (!m_needsPersisting)
        return;
#if defined(ENABLE_URLSTORE_DEBUG)
    qDebug() << "UrlStore:" << __FUNCTION__ << "urlstore.txt"<<endl;
#endif
    // save thumbnails first
    for (int i = 0; i < m_list.size(); ++i) {
        UrlItem* item = m_list[i];
        // save if new thumbnail is available
        if (item->thumbnailAvailable() && item->m_thumbnailChanged) {
             item->m_thumbnailChanged = false;
             // first time?
             if (!item->m_thumbnailPath.size())
                 item->m_thumbnailPath = QString::number(item->m_lastAccess) + ".png";
#if defined(ENABLE_URLSTORE_DEBUG)
            qDebug() << item->m_thumbnailPath;
#endif
            item->thumbnail()->save(thumbnailDir() + item->m_thumbnailPath);
        }
    }
    // save url store
    // version
    // number of items
    // url, refcount, lastaccess
    QFile store(thumbnailDir() + "urlstore.txt");
    if (store.open(QFile::WriteOnly | QIODevice::Truncate)) {
        QDataStream out(&store);
        out<<currentVersion<<m_list.size();
        for (int i = 0; i < m_list.size(); ++i) {
            UrlItem* item = m_list.at(i);
            out<<item->m_url.toString()<<item->m_title<<item->m_refcount<<item->m_lastAccess<<item->m_thumbnailPath;
        }
        store.close();
    } 
    m_needsPersisting = false;
}

void UrlStore::accessed(const QUrl& url, const QString& title, QImage* thumbnail)
{
    QString accessedHostAndPath = url.host() + url.path();
    int found = -1;
#if defined(ENABLE_URLSTORE_DEBUG)
    qDebug() << "UrlStore:" << __FUNCTION__ << accessedHost;
#endif
    for (int i = 0; i < m_list.size(); ++i) {
        UrlItem* item = m_list[i];
        if (matchUrls(item->m_url.host() + item->m_url.path(), accessedHostAndPath)) {
            item->m_refcount++;
            item->m_lastAccess = QDateTime::currentDateTime().toTime_t();
            // move it up if needed
            int j = i;
            // '<=' is for the last access sorting, recently used items move up
            while (--j >= 0 && item->m_refcount >= m_list.at(j)->m_refcount);
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
        while (--i >= 0 && m_list.at(i)->m_refcount == 1);
        m_list.insert(++i, new UrlItem(url, title, thumbnail));
    } else if (thumbnail) {
        // add thumbnail if not there yet
        m_list[found]->setThumbnail(thumbnail);
    }
#if defined(ENABLE_URLSTORE_DEBUG)
    for (int i = 0; i < m_list.size(); ++i)
        qDebug()<<m_list.at(i)->m_url.toString()<<" "<<m_list.at(i)->m_refcount;
#endif
    m_needsPersisting = true;
    QTimer::singleShot(2000, this, SLOT(externalize()));
}

bool UrlStore::contains(const QString& url)
{
    for (int i = 0; i < m_list.size(); ++i)
        if (m_list.at(i)->m_url.toString() == url)
            return true;
    return false;
}

QString UrlStore::match(const QString& url)
{
    if (!url.size())
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

bool UrlStore::matchUrls(const QString& url1, const QString& url2) 
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

