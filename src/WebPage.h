#ifndef WebPage_h
#define WebPage_h

#include <qwebpage.h>

class WebPage : public QWebPage {
    Q_OBJECT

public:
    WebPage(QObject* parent = 0);
    virtual QWebPage* createWindow(QWebPage::WebWindowType);

private:

};

#endif
