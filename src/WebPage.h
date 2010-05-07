#ifndef WebPage_h
#define WebPage_h

#include <qwebpage.h>

class BrowsingView;

class WebPage : public QWebPage {
    Q_OBJECT

public:
    WebPage(QObject* parent = 0, BrowsingView* ownerView = 0);
    virtual QWebPage* createWindow(QWebPage::WebWindowType);

private:
    BrowsingView* m_ownerView;
};

#endif
