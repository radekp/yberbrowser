#ifndef BookmarkStore_h_
#define BookmarkStore_h_

#include <QObject>
#include <QList>
#include <QUrl>
#include <QIcon>
#include "UrlItem.h"

typedef QList<UrlItem*> BookmarkList;

class BookmarkStore : public QObject {
    Q_OBJECT
public:
    static BookmarkStore* instance();    

    void add(const QUrl& url, const QString& title, QIcon favicon);
    void remove(const QString& url);
    const UrlList& list() const { return m_list; }

private:
    BookmarkStore();
    ~BookmarkStore();

private Q_SLOTS:
    void externalize();

private:
    UrlList m_list;
    bool m_needsPersisting;
};

#endif

