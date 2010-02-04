#include "UrlItem.h"
#include "UrlStore.h"
#include <QDateTime>
#include <QImage>

UrlItem::UrlItem()
    : m_refcount(0)
    , m_lastAccess(0)
    , m_thumbnailChanged(false)
    , m_thumbnail(0)
{
}

UrlItem::UrlItem(const QUrl& url, const QString& title, QImage* thumbnail)
    : m_url(url)
    , m_title(title)
    , m_refcount(1)
    , m_lastAccess(QDateTime::currentDateTime().toTime_t())
    , m_thumbnailChanged(false)
    , m_thumbnail(thumbnail)
{
}

UrlItem::~UrlItem()
{
    delete m_thumbnail;
}

void UrlItem::setThumbnail(QImage* thumbnail) 
{ 
    m_thumbnail = thumbnail; 
    m_thumbnailChanged = true;
    emit thumbnailChanged();
}

QImage* UrlItem::thumbnail()
{
    if (m_thumbnail)
        return m_thumbnail;
    if (!m_thumbnailPath.size())
        return 0;
    // load thumbnail
    m_thumbnail = new QImage(UrlStore::thumbnailDir() + m_thumbnailPath);
    return m_thumbnail;
}


