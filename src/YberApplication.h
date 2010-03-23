#ifndef YberApplication_h
#define YberApplication_h

#include "yberconfig.h"

#include "ApplicationWindow.h"

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

private:
    Q_DISABLE_COPY(YberApplication);

    bool m_isFullscreen;
    bool m_isTileCacheEnabled;
    QScopedPointer<ApplicationWindow> appwin;
};





#endif
