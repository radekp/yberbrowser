#include "BookmarkStore.h"
#include <QImage>
#include <QPixmap>
#include <QTimer>

#include "Helpers.h"

#include <QDebug>

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
    internalizeUrlList(m_list, "bookmarkstore.txt");
    if (!m_list.size()) {
        // FIXME move icons out of the res file. 
        add(QUrl("http://www.yahoo.com/"), "Yahoo!", QIcon(":/data/_yberbrowser/yahoo.ico") );
        add(QUrl("http://www.msn.com/"), "MSN.com", QIcon(":/data/_yberbrowser/msn.ico") );
        add(QUrl("http://www.wikipedia.org/"), "Wikipedia", QIcon(":/data/_yberbrowser/wikipedia.ico") );
        add(QUrl("http://www.facebook.com/"), "Facebook", QIcon(":/data/_yberbrowser/facebook.ico") );
        add(QUrl("http://twitter.com/"), "Twitter", QIcon(":/data/_yberbrowser/twitter.ico") );
        add(QUrl("http://www.google.com/"), "Google", QIcon(":/data/_yberbrowser/google.ico") );
    }
}

// FIXME: this is a singleton, dont get properly deleted
BookmarkStore::~BookmarkStore()
{
    externalize();
    for (int i = 0; i < m_list.size(); ++i)
        delete m_list.takeAt(i);
}

void BookmarkStore::add(const QUrl& url, const QString& title, QIcon favicon)
{
    for (int i = 0; i < m_list.size(); ++i) {
        if (m_list[i]->m_url == url) {
            // move it to the beginning of the queue
            m_list.move(i, 0);
            return;
        }
    }
    // fixme webkit proives empty icons
    QImage* image = new QImage(favicon.pixmap(QSize(16,16)).toImage());;
    if (image->size() == QSize(0, 0)) {
        delete image;
        image = 0;
    }

    m_list.insert(0, new UrlItem(url, title, image));
    m_needsPersisting = true;
    QTimer::singleShot(5000, this, SLOT(externalize()));
}

void BookmarkStore::remove(const QString& /*url*/)
{
}

void BookmarkStore::externalize()
{
    if (!m_needsPersisting)
        return;
    externalizeUrlList(m_list, "bookmarkstore.txt");
    m_needsPersisting = false;
}

