#include "UrlStore.h"
#include <QDataStream>
#include <QFile>
#include <QDateTime>
#include <iostream>
#include <qdebug.h>

static uint currentVersion = 1;

UrlItem::UrlItem()
    : m_refcount(0)
    , m_lastAccess(0)
{
}

UrlItem::UrlItem(const QString& host)
    : m_host(host)
    , m_refcount(1)
    , m_lastAccess(QDateTime::currentDateTime().toTime_t())
{
}

UrlItem::UrlItem(const UrlItem& item) 
{
    m_host = item.m_host;
    m_refcount = item.m_refcount;
    m_lastAccess = item.m_lastAccess;
}

UrlStore::UrlStore()
{
    internalize();
}

UrlStore::~UrlStore()
{
    externalize();
}

void UrlStore::internalize()
{
    // read url store
    // version
    // number of items
    // url, refcount, lastaccess
    qDebug()<<"UrlStore: internalize urlstore.txt"<<endl;
    QFile store("urlstore.txt");
    if (store.open(QFile::ReadWrite)) {
        QDataStream in(&store);
        uint version;
        in>>version;
        if (version == currentVersion) {
            int count;
            in>>count;
            for (int i = 0; i < count; ++i) {
                UrlItem item;
                in>>item.m_host>>item.m_refcount>>item.m_lastAccess;
                m_list.append(item);
            }
        }
        store.close();
    } 

    if (!m_list.size()) {
        qDebug()<<"UrlStore: no url store, use default values";
        // init urlstore with some popular urls. prefer non-www for to save space
        m_list.append(UrlItem("google.com"));
        m_list.append(UrlItem("cnn.com"));
        m_list.append(UrlItem("news.bbc.co.uk"));
        m_list.append(UrlItem("news.google.com"));
        m_list.append(UrlItem("msn.com"));
        m_list.append(UrlItem("nytimes.com"));
    }
}

void UrlStore::externalize()
{
    qDebug()<<"UrlStore: externalize urlstore.txt"<<endl;
    // save url store
    // version
    // number of items
    // url, refcount, lastaccess
    QFile store("urlstore.txt");
    if (store.open(QFile::WriteOnly | QIODevice::Truncate)) {
        QDataStream out(&store);
        out<<currentVersion<<m_list.size();
        for (int i = 0; i < m_list.size(); ++i) {
            UrlItem item = m_list.at(i);
            out<<item.m_host<<item.m_refcount<<item.m_lastAccess;
        }
        store.close();
    } 
}

void UrlStore::accessed(const QUrl& prettyUrl)
{
    QString host = prettyUrl.host();
    bool found = false;
    
    qDebug()<<"UrlStore: accessed: "<<host;

    for (int i = 0; i < m_list.size(); ++i) {
        UrlItem* item = &m_list[i];
        if (matchUrls(item->m_host, host)) {
            item->m_refcount++;
            // move it up if needed
            int j = i;
            // '<=' is for the last access sorting, recently used items move up
            while (--j >= 0 && item->m_refcount >= m_list.at(j).m_refcount);
            // position adjusting and check whether we really moved
            if (++j != i) 
                m_list.move(i, j);
            found = true;
            break;
        }
    }
    
    if (!found) {
        UrlItem newItem(host);
        // insert to the top of the 1 refcount items. recently used sort
        int i = m_list.size();
        while (--i >= 0 && m_list.at(i).m_refcount == 1);
        m_list.insert(++i, newItem);
    }

    for (int i = 0; i < m_list.size(); ++i)
        qDebug()<<m_list.at(i).m_host<<" "<<m_list.at(i).m_refcount;
}

QString UrlStore::match(const QString& str)
{
    if (!str.size())
        return QString();
    for (int i = 0; i < m_list.size(); ++i) {
        // do a very simply startWith matching first.
        QString host = m_list.at(i).m_host;
        if (host.startsWith(str))
            return host;
        else if (host.startsWith("www.")) {
            host = host.mid(4);
            if (host.startsWith(str))
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




