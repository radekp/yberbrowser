#include "Helpers.h"
#include <QFileInfo>
#include "Settings.h"
#include <QImage>
#include <QPropertyAnimation>
#include <QGraphicsWidget>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QPen>
#include <QPainter>
#include <QLinearGradient>

#include <QDebug>

namespace {
static uint s_currentVersion = 2;
static int s_maxUrlItems = 50;
static int s_notificationHeight = 50;
}

class NotificationWidget : public QGraphicsWidget {
    Q_OBJECT
public: 
    static void show(const QString& text, QGraphicsWidget* parent);

private:
    NotificationWidget(const QString& text, QGraphicsItem* parent, Qt::WindowFlags wFlags = 0);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void doShow();

private Q_SLOTS:
    void startAnimation();
    void animFinished();

private:
    QString m_text;
    bool m_scrollingIn;
};

static NotificationWidget* s_notification = 0;

void NotificationWidget::show(const QString& text, QGraphicsWidget* parent)
{
    if (s_notification)
        return;

    s_notification = new NotificationWidget(text, parent);
    s_notification->doShow();
}

NotificationWidget::NotificationWidget(const QString& text, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_text(text)
    , m_scrollingIn(true)
{
    setOpacity(0.8);
}

void NotificationWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(Qt::black);
    painter->setBrush(Qt::black);
    painter->drawRect(rect());

    QFont f("Times", 14);
    painter->setFont(f);
    painter->setPen(Qt::green);
    painter->drawText(rect(), Qt::AlignHCenter|Qt::AlignVCenter, m_text);
}

void NotificationWidget::doShow()
{
    QTimer::singleShot(0, this, SLOT(startAnimation()));
}

void NotificationWidget::startAnimation()
{
    QRectF r(parentWidget()->rect()); r.setTop(r.bottom() - s_notificationHeight); 
    QRectF hidden(r); hidden.setTop(r.bottom());

    QPropertyAnimation* anim = new QPropertyAnimation(this, "geometry");
    anim->setDuration(800);
    anim->setStartValue(m_scrollingIn ? hidden : r);
    anim->setEndValue(m_scrollingIn ? r : hidden);

    anim->setEasingCurve(m_scrollingIn ? QEasingCurve::OutBack : QEasingCurve::InBack);
    anim->start();

    if (m_scrollingIn)
        setGeometry(hidden);
    connect(anim, SIGNAL(finished()), this, SLOT(animFinished()));
}

void NotificationWidget::animFinished()
{
    if (m_scrollingIn) {
        m_scrollingIn = false;
        QTimer::singleShot(800, this, SLOT(startAnimation()));
    } else {
        delete s_notification;
        s_notification = 0;
        // "this" is invalid at this point
    }
}

void notification(const QString& text, QGraphicsWidget* parent)
{
    NotificationWidget::show(text, parent);
}

QUrl urlFromUserInput(const QString& string)
{
    QString input(string);
    QFileInfo fi(input);
    if (fi.exists() && fi.isRelative())
        input = fi.absoluteFilePath();

    return QUrl::fromUserInput(input);
}

void internalizeUrlList(UrlList& list, const QString& fileName)
{
    // read url store
    // version
    // number of items
    // url, refcount, lastaccess
    QFile store(Settings::instance()->privatePath() + fileName);

    if (store.open(QFile::ReadWrite)) {
        QDataStream in(&store);
        uint version;
        in>>version;
        if (version == s_currentVersion) {
            int count;
            in>>count;
            for (int i = 0; i < count; ++i) {
                UrlItem* item = new UrlItem();
                QString url;
                QString path;
                in>>url; item->m_url = url;
                in>>item->m_title>>item->m_refcount>>item->m_lastAccess>>path;
                item->setThumbnailPath(path);
                list.append(item);
            }
        }
        store.close();
    } 
}

void externalizeUrlList(const UrlList& list, const QString& fileName)
{
    int count = qMin(list.size(), s_maxUrlItems);
    // save thumbnails first
    for (int i = 0; i < count; ++i) {
        UrlItem* item = list[i];
        // save if new thumbnail is available
        if (item->thumbnailAvailable() && item->m_thumbnailChanged) {
            item->m_thumbnailChanged = false;
            item->thumbnail()->save(Settings::instance()->privatePath() + item->thumbnailPath());
        }
    }
    // save url store
    // version
    // number of items
    // url, refcount, lastaccess
    QFile store(Settings::instance()->privatePath() + fileName);
    if (store.open(QFile::WriteOnly | QIODevice::Truncate)) {
        QDataStream out(&store);
        out<<s_currentVersion<<count;
        for (int i = 0; i < count; ++i) {
            UrlItem* item = list.at(i);
            out<<item->m_url.toString()<<item->m_title<<item->m_refcount<<item->m_lastAccess<<item->thumbnailPath();
        }
        store.close();
    } 
}

#include "Helpers.moc"
