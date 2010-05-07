#include "WebPage.h"
#include "BrowsingView.h"
#include "WebView.h"
#include "YberApplication.h"

#include <QDebug>

WebPage::WebPage(QObject* parent, BrowsingView* ownerView)
    : QWebPage(parent)
    , m_ownerView(ownerView)
{
    CookieJar* jar = YberApplication::instance()->cookieJar();
    // setCookieJar changes the parent of the passed jar ;(
    // So we need to preserve it
    QObject* oldParent = jar->parent();
    networkAccessManager()->setCookieJar(jar);
    jar->setParent(oldParent);
}

QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    if (!m_ownerView)
        return 0;

    WebView* webView = m_ownerView->newWindow();

    if (!webView)
        return 0;

    return webView->page();
}
