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
class YberApplication;
class WebViewportItem;
class PannableViewport;
class ApplicationWindow;
class TabSelectionView;
class HomeView;
class BackingStoreVisualizerWidget;
class ProgressWidget;
class QAction;
class AutoScrollTest;
class QToolBar;

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

    WebView* newWindow();
    void setActiveWindow(WebView* webView);
    void destroyWindow(WebView* webView);
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
    void urlEditfocusChanged(bool);
    void urlChanged(const QUrl& url);

    void updateHistoryStore();

    void setLoadInProgress(bool);
    void loadStarted();
    void loadFinished(bool success);

    void updateURL();

    void toggleFullScreen();

    void createHomeView();
    void deleteHomeView();

    void createTabSelectionView();
    void deleteTabSelectionView();
    void toggleTabSelectionView();

    void prepareForResize();

    void startAutoScrollTest();
    void finishedAutoScrollTest();

private:
    Q_DISABLE_COPY(BrowsingView);
    YberWidget* createNavigationToolBar();
    void connectSignals(WebView* currentView, WebView* oldView);
    void updateViews();

    void toggleStopBackIcon();

#if !USE_DUI
    QMenuBar* createMenu(QWidget* parent);
#endif

#if !USE_DUI
    ApplicationWindow* m_appWin;
    YberWidget* m_centralWidget;
#endif

    QToolBar* m_toolbar;
    WebView* m_activeWebView;
    PannableViewport* m_browsingViewport;
    WebViewportItem* m_webInteractionProxy;
    TabSelectionView* m_tabSelectionView;
    HomeView* m_homeView;
    UrlEditWidget* m_urlEdit;
    ProgressWidget* m_progressBox;
    QSizeF m_sizeBeforeResize;
    QString m_lastEnteredText;
    QAction* m_stopbackAction;
    bool m_loadIndProgress;
    AutoScrollTest* m_autoScrollTest;
    QList<WebView*> m_windowList;
};


#endif
