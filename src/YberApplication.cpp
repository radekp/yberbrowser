#include <QUrl>
#include <QNetworkProxyFactory>

#include "YberApplication.h"
#include "BrowsingView.h"
#include "Settings.h"
#include "Helpers.h"
#include "EnvHttpProxyFactory.h"

#include "ApplicationWindow.h"


#include <QDebug>
YberApplication::YberApplication(int & argc, char ** argv)
    : YberApplicationBase(argc, argv)
    , m_cookieJar(0)
{
    bool useSystemConf = true;

    QList<QNetworkProxy> proxylist = QNetworkProxyFactory::systemProxyForQuery();

    if (proxylist.count() == 1) {
        QNetworkProxy proxy = proxylist.first();
        if (proxy == QNetworkProxy::NoProxy || proxy == QNetworkProxy::DefaultProxy) {
            EnvHttpProxyFactory* pf = new EnvHttpProxyFactory();
            if (pf->initFromEnvironment()) {
                QNetworkProxyFactory::setApplicationProxyFactory(pf);
                useSystemConf = false;
            }
        }
    }
    if (useSystemConf)
        QNetworkProxyFactory::setUseSystemConfiguration(true);
}


YberApplication::~YberApplication()
{
    delete m_cookieJar;
}

void YberApplication::start()
{
    if (!appwin) {
        appwin.reset(new ApplicationWindow());
        if (Settings::instance()->isFullScreen())
            appwin->showFullScreen();
        else
            appwin->show();
    }
}

void YberApplication::createMainView(const QUrl& url)
{
    BrowsingView* page = new BrowsingView(*this);

#if USE_DUI
    page->appear(appwin.data(), DuiSceneWindow::DestroyWhenDone);
#else
    page->appear(appwin.data());
#endif

    if (url.isEmpty())
        page->showHomeView();
    else
        page->load(url);
}

CookieJar* YberApplication::cookieJar() const
{
    if (!m_cookieJar)
        m_cookieJar = new CookieJar;
    return m_cookieJar;
}
