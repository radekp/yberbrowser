#ifndef BrowsingView_h
#define BrowsingView_h

#include "yberconfig.h"

#if USE_DUI
#include <DuiApplicationPage>

typedef DuiApplicationPage BrowsingViewBase;
typedef DuiWidget YberWidget;
class DuiTextEdit;
#else
#include <QGraphicsWidget>
#include <QUrl>
typedef QGraphicsWidget BrowsingViewBase;
typedef QGraphicsWidget YberWidget;

class QMenuBar;
typedef QMenuBar MenuBar;
class AutoSelectLineEdit;
#endif

class WebView;
class WebPage;
class YberApplication;
class WebViewportItem;
class PannableViewport;
class ApplicationWindow;
class HomeView;
class BackingStoreVisualizerWidget;
class ProgressWidget;
class QAction;
class AutoScrollTest;

class BrowsingView : public BrowsingViewBase
{
    Q_OBJECT
#if USE_DUI
    typedef DuiTextEdit UrlEditWidget;
#else
    typedef AutoSelectLineEdit UrlEditWidget;
#endif

public:
    BrowsingView(YberApplication&, QGraphicsItem* parent = 0);
    ~BrowsingView();

#if !USE_DUI
    ApplicationWindow* applicationWindow() { return m_appWin; }
    void appear(ApplicationWindow* window);
    YberWidget* centralWidget()  { return m_centralWidget; }
    PannableViewport* pannableViewport() { return m_browsingViewport; }
    void showHomeView();
    void hideHomeView();
#endif
public Q_SLOTS:
    void load(const QUrl&);
#if !USE_DUI
    void setTitle(const QString&);
#endif

protected:
    void resizeEvent(QGraphicsSceneResizeEvent* event);

protected Q_SLOTS:
    void addBookmark();
    void stopLoad();
    void pageBack();
    void changeLocation();
    void urlTextEdited(const QString& newText);
    void urlChanged(const QUrl& url);

    void updateHistoryStore();

    void setLoadInProgress(bool);
    void loadStarted();
    void loadFinished(bool success);

    void updateURL();
    void showFPSChanged(bool);
    void showTilesChanged(bool);

    BrowsingView* newWindow(const QString& url);

    void toggleFullScreen();

    void createHomeView();
    void deleteHomeView();
    void toggleHomeView();

    void prepareForResize();

    void startAutoScrollTest();
    void finishedAutoScrollTest();

private:
    Q_DISABLE_COPY(BrowsingView);
    YberWidget* createNavigationToolBar();
    YberWidget* navigationToolbar();
    void setFPSCalculation(bool fpsOn);
    void connectSignals();
    void updateHomeView();

    void toggleStopBackIcon();

#if !USE_DUI
    QMenuBar* createMenu(QWidget* parent);
#endif

#if !USE_DUI
    ApplicationWindow *m_appWin;
    YberWidget* m_centralWidget;
#endif

    WebView* m_webView;
    WebPage* m_page;
    PannableViewport* m_browsingViewport;
    BackingStoreVisualizerWidget* m_backingStoreVisualizer;
    HomeView* m_homeView;
    UrlEditWidget* m_urlEdit;
    ProgressWidget* m_progressBox;
    QSizeF m_sizeBeforeResize;
    QString m_lastEnteredText;
    QAction* m_stopbackAction;
    bool m_loadIndProgress;
    AutoScrollTest* m_autoScrollTest;
};


#endif
