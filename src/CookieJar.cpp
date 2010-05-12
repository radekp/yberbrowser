#include "CookieJar.h"
#include "Settings.h"

#include <QFile>
#include <QDateTime>
#include <QTimerEvent>

const quint8 cookieFileVersion = 1;

CookieJar::CookieJar(QObject* parent)
    : QNetworkCookieJar(parent)
    , m_cookiesChanged(false)
{
    load();
    // save cookies every 2 minutes
    m_cookieSavingTimer.start(1000 * 60 * 2, this);
}

CookieJar::~CookieJar()
{
    save();
}

void CookieJar::save()
{
    expireCookies();
    if (!m_cookiesChanged)
        return;

    QByteArray cookieData;
    QDataStream stream(&cookieData, QIODevice::WriteOnly);
    stream << cookieFileVersion;
    QList<QNetworkCookie> cookies = allCookies();

    QMutableListIterator<QNetworkCookie> it(cookies);
    while (it.hasNext()) {
        if (it.next().isSessionCookie())
            it.remove();
    }

    stream << qint32(cookies.count());
    foreach (const QNetworkCookie& cookie, cookies)
        stream << cookie.toRawForm();

    QString cookieFileName = Settings::instance()->cookieFilePath();
    QFile cookieFile(cookieFileName + ".tmp");
    if (!cookieFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;

    cookieFile.write(cookieData);
    cookieFile.close();
    // ### use atomic rename
    QFile::remove(cookieFileName);
    cookieFile.rename(cookieFileName);
    m_cookiesChanged = false;
}

void CookieJar::load()
{
    QFile cookieFile(Settings::instance()->cookieFilePath());
    if (!cookieFile.open(QIODevice::ReadOnly))
        return;

    QDataStream stream(&cookieFile);
    quint8 version;
    stream >> version;
    if (version != cookieFileVersion)
        return;

    QList<QNetworkCookie> cookies;
    qint32 count;
    stream >> count;

#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    cookies.reserve(count);
#endif
    for (int i = 0; i < count && !stream.atEnd(); ++i) {
        QByteArray rawCookie;
        stream >> rawCookie;
        cookies += QNetworkCookie::parseCookies(rawCookie);
    }

    setAllCookies(cookies);
    m_cookiesChanged = false;

    expireCookies();
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    m_cookiesChanged = true;
    return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
}

void CookieJar::timerEvent(QTimerEvent* ev)
{
    if (ev->timerId() == m_cookieSavingTimer.timerId()) {
        save();
        return;
    }
    return QObject::timerEvent(ev);
}

void CookieJar::expireCookies()
{
    const QList<QNetworkCookie>& currentCookies = allCookies();
    QList<QNetworkCookie> newCookies;
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    newCookies.reserve(currentCookies.count());
#endif    
    QDateTime now = QDateTime::currentDateTime();
    foreach (const QNetworkCookie& cookie, currentCookies) {
        if (!cookie.isSessionCookie() && cookie.expirationDate() < now) {
            m_cookiesChanged = true;
            continue;
        }
        newCookies += cookie;
    }
    setAllCookies(newCookies);
}
