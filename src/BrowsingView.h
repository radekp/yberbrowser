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
class HistoryView;
class BackingStoreVisualizerWidget;
class ProgressWidget;

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


#if !USE_DUI
    ApplicationWindow* applicationWindow() { return m_appWin; }
    void appear(ApplicationWindow* window);
    YberWidget* centralWidget()  { return m_centralWidget; }
#endif
public Q_SLOTS:
    void load(const QUrl&);
#if !USE_DUI
    void setTitle(const QString&);
#endif



protected:
    void resizeEvent(QGraphicsSceneResizeEvent* event);

protected Q_SLOTS:
    void changeLocation();
    void urlTextEdited(const QString& newText);
    void urlChanged(const QUrl& url);

    void setLoadInProgress(bool);
    void loadStarted();
    void loadFinished(bool success);

    void updateURL();
    void showFPSChanged(bool);
    void showTilesChanged(bool);

    BrowsingView* newWindow(const QString& url);

    void toggleFullScreen();

    void toggleHistoryView();
    void deleteHistoryView();

    void prepareForResize();

private:
    Q_DISABLE_COPY(BrowsingView);
    YberWidget* createNavigationToolBar();
    void setFPSCalculation(bool fpsOn);
    void connectSignals();
    void hideHistoryView();
    void updateHistoryView();
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
    HistoryView* m_historyView;
    UrlEditWidget* m_urlEdit;
    ProgressWidget* m_progressBox;
    QSizeF m_sizeBeforeResize;
};


#endif
