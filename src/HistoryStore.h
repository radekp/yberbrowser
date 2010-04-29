#ifndef HistoryStore_h_
#define HistoryStore_h_

#include <QObject>
#include <QList>
#include <QUrl>
#include "UrlItem.h"

class HistoryStore : public QObject {
    Q_OBJECT
public:
    static HistoryStore* instance();    

    void accessed(const QUrl& url, const QString& title, QImage* thumbnail);
    bool contains(const QString& url);
    QString match(const QString& url);
    UrlList* list() { return &m_list; }

private:
    HistoryStore();
    ~HistoryStore();

    void internalize();
    bool matchUrls(const QString& url1, const QString& url2);

private Q_SLOTS:
    void externalize();

private:
    UrlList m_list;
    bool m_needsPersisting;
};

#endif

