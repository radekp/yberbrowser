#include "WebPage.h"

WebPage::WebPage(QObject* parent)
    : QWebPage(parent)
{}

QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    Q_ASSERT(0);
    // FIXME
    return 0;
}
