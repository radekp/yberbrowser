#ifndef UrlStore_h_
#define UrlStore_h_

#include <QObject>
#include <QList>
#include <QUrl>

class UrlItem {
public:
    UrlItem();
    UrlItem(const QString& host);
    UrlItem(const UrlItem& item);

    QString m_host;
    uint m_refcount;
    uint m_lastAccess;
};

typedef QList<UrlItem> UrlList;

class UrlStore : public QObject {
    Q_OBJECT

public:
    UrlStore();
    ~UrlStore();
    
    void accessed(const QUrl& url);
    QString match(const QString& str);    

private:
    void internalize();
    void externalize();
    bool matchUrls(const QString& url1, const QString& url2);

    UrlList m_list;
};

#endif

