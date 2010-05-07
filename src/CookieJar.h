#ifndef CookieJar_h_
#define CookieJar_h_

#include <QNetworkCookieJar>
#include <QBasicTimer>

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    CookieJar(QObject* parent = 0);
    virtual ~CookieJar();

    void save();
    void load();

    virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

protected:
    virtual void timerEvent(QTimerEvent* ev);

private:
    void expireCookies();
    QBasicTimer m_cookieSavingTimer;
    bool m_cookiesChanged;
};

#endif

