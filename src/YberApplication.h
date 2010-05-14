#ifndef YberApplication_h
#define YberApplication_h

#include "yberconfig.h"

#include "ApplicationWindow.h"
#include "CookieJar.h"

#if USE_DUI
#include <DuiApplication>
typedef DuiApplication YberApplicationBase;
#else
#include <QApplication>
typedef QApplication YberApplicationBase;
#endif

class YberApplication : public YberApplicationBase
{
public:
    explicit YberApplication(int & argc, char ** argv);
    ~YberApplication();

    void start();

    void createMainView(const QUrl& url);

    bool isFullscreen() const { return m_isFullscreen; }
    bool isTileCacheEnabled() const { return m_isTileCacheEnabled; }

    CookieJar* cookieJar() const;

    static YberApplication* instance()
    { return static_cast<YberApplication*>(QCoreApplication::instance()); }

private:
    Q_DISABLE_COPY(YberApplication);

    bool m_isFullscreen;
    bool m_isTileCacheEnabled;
    ApplicationWindow *appwin;
    mutable CookieJar* m_cookieJar;
};





#endif
