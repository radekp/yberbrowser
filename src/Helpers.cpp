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
#include <QNetworkRequest>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebframe.h>

#include <QDebug>

namespace {
static uint s_currentVersion = 2;
static int s_maxUrlItems = 50;
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
{
}

void NotificationWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(Qt::black);
    painter->setBrush(QColor(80, 80, 80));

    painter->drawRect(rect());
 
    QFont f("Times", 18);
    painter->setFont(f);
    painter->setPen(Qt::white);
    painter->drawText(rect(), Qt::AlignHCenter|Qt::AlignVCenter, m_text);
}

void NotificationWidget::doShow()
{
    QRectF r(parentWidget()->rect());
    QFont f("Times", 18);
    int height = QFontMetrics(f).height() + 2;
    
    r.setTop(r.center().y()/2 - height/2);
    r.setHeight(height);
    setGeometry(r);
    QTimer::singleShot(800, this, SLOT(startAnimation()));
}

void NotificationWidget::startAnimation()
{
    QPropertyAnimation* anim = new QPropertyAnimation(this, "opacity");
    anim->setDuration(400);
    anim->setStartValue(1);
    anim->setEndValue(0);

    anim->setEasingCurve(QEasingCurve::Linear);
    anim->start();
    connect(anim, SIGNAL(finished()), this, SLOT(animFinished()));
}

void NotificationWidget::animFinished()
{
    delete s_notification;
    s_notification = 0;
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
