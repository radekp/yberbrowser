#include "WebPage.h"
#include "BrowsingView.h"
#include "WebView.h"

#include <QDebug>

WebPage::WebPage(QObject* parent, BrowsingView* ownerView)
    : QWebPage(parent)
    , m_ownerView(ownerView)
{}

QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    if (!m_ownerView)
        return 0;

    WebView* webView = m_ownerView->newWindow();

    if (!webView)
        return 0;

    return webView->page();
}
