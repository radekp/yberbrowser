#include "WebPage.h"

#include <QDebug>

WebPage::WebPage(QObject* parent)
    : QWebPage(parent)
{}

QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    return 0;
}
