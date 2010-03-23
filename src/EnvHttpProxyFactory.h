#ifndef EnvHttpProxyFactory_h_
#define EnvHttpProxyFactory_h_

#include <QNetworkProxyFactory>
#include <QList>

class EnvHttpProxyFactory : public QNetworkProxyFactory
{
public:
    EnvHttpProxyFactory();

    bool initFromEnvironment();

    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery & query = QNetworkProxyQuery());

private:
    QList<QNetworkProxy> m_httpProxy;
    QList<QNetworkProxy> m_httpsProxy;

};

#endif
