#ifndef UrlItem_h_
#define UrlItem_h_

#include <QObject>
#include <QUrl>
#include <QString>
#include <QList>

class QImage;

class UrlItem : public QObject {
    Q_OBJECT
public:
    UrlItem();
    UrlItem(const QUrl& url, const QString& title, QImage* image = 0, void* context = 0);
    ~UrlItem();

    bool thumbnailAvailable() const { return m_thumbnail != 0; }
    void setThumbnail(QImage* thumbnail);
    QImage* thumbnail();
    void setThumbnailPath(const QString& path) { m_thumbnailPath = path; }
    QString thumbnailPath();

Q_SIGNALS:
    void thumbnailChanged();

public:
    QUrl m_url;
    QString m_title;
    uint m_refcount;
    uint m_lastAccess;
    bool m_thumbnailChanged;
    void* m_context; // fake item, until i figure out how to tranform this class to a more generic one

private:
    QImage* m_thumbnail;
    QString m_thumbnailPath;
};

typedef QList<UrlItem*> UrlList;

#endif

