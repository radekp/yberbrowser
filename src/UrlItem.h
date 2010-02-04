#ifndef UrlItem_h_
#define UrlItem_h_

#include <QObject>
#include <QUrl>
#include <QString>

class QImage;

class UrlItem : public QObject {
    Q_OBJECT
public:
    UrlItem();
    UrlItem(const QUrl& url, const QString& title, QImage* image = 0);
    ~UrlItem();

    bool thumbnailAvailable() const { return m_thumbnail != 0; }
    void setThumbnail(QImage* thumbnail);
    QImage* thumbnail();

Q_SIGNALS:
    void thumbnailChanged();

public:
    QUrl m_url;
    QString m_title;
    uint m_refcount;
    uint m_lastAccess;
    QString m_thumbnailPath;
    bool m_thumbnailChanged;

private:
    QImage* m_thumbnail;
};

#endif

