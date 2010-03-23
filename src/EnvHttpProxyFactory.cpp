#include <QUrl>

#include "EnvHttpProxyFactory.h"
#include "Helpers.h"

EnvHttpProxyFactory::EnvHttpProxyFactory()
{

}

bool EnvHttpProxyFactory::initFromEnvironment()
{
    bool result = false;

    QUrl proxyUrl = urlFromUserInput(qgetenv("http_proxy"));
    if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
        int proxyPort = (proxyUrl.port() > 0) ? proxyUrl.port() : 8080;
        m_httpProxy << QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyPort);
        result = true;
    } else {
        m_httpProxy << QNetworkProxy::NoProxy;
    }

    proxyUrl = urlFromUserInput(qgetenv("https_proxy"));
    if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
        int proxyPort = (proxyUrl.port() > 0) ? proxyUrl.port() : 8080;
        m_httpsProxy << QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyPort);
        result = true;
    } else {
        m_httpsProxy << QNetworkProxy::NoProxy;
    }
    return result;
}

QList<QNetworkProxy> EnvHttpProxyFactory::queryProxy(const QNetworkProxyQuery & query)
{

    QString protocol = query.protocolTag().toLower();
    if (protocol == QLatin1String("http"))
        return m_httpProxy;
    else if (protocol == QLatin1String("https"))
        return m_httpsProxy;

    QList<QNetworkProxy> result;
    result << QNetworkProxy::NoProxy;
    return result;
}
