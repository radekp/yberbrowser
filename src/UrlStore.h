#ifndef UrlStore_h_
#define UrlStore_h_

#include <QObject>
#include <QList>
#include <QUrl>
#include "UrlItem.h"

typedef QList<UrlItem*> UrlList;

class UrlStore : public QObject {
    Q_OBJECT
public:
    UrlStore();
    ~UrlStore();
    
    void accessed(const QUrl& url, const QString& title, QImage* thumbnail);
    bool contains(const QString& url);
    QString match(const QString& url);
    UrlList& list() { return m_list; }

    static QString thumbnailDir() {
        return s_thumbnailDir;
    }

    static void setThumbnailDir(const QString& dir) {
        s_thumbnailDir = dir;
    }

private:
    void internalize();
    bool matchUrls(const QString& url1, const QString& url2);

private Q_SLOTS:
    void externalize();

private:
    UrlList m_list;
    bool m_needsPersisting;

    static QString s_thumbnailDir;
};

#endif

