#ifndef MainWindow_h_
#define MainWindow_h_

#include <QMainWindow>
#include <QUrl>
#include <qgraphicswebview.h>
#include <qwebpage.h>

class QGraphicsScene;
class QLineEdit;
class WebPage;
class WebView;
class MainView;
class QNetworkProxy;



struct Settings
{
    bool m_disableToolbar;
    bool m_disableTiling;
    bool m_useGL;
    Settings()
        : m_disableToolbar(false)
        , m_disableTiling(false)
        , m_useGL(false)
    {}

    Settings(bool disableToolbar, bool disableTiling, bool useGL)
        : m_disableToolbar(disableToolbar)
        , m_disableTiling(disableTiling)
        , m_useGL(useGL)
    {}

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QNetworkProxy* m_proxy = 0, Settings settings = Settings());
    ~MainWindow();
    void init();

    void load(const QString& url);

    WebPage* createWebPage();
    MainWindow* createWindow();

    QWebPage* page() const;

    void disableHildonDesktopCompositing();

    MainView* view();

    void changeLocation();

    void loadFinished(bool);

    static QUrl urlFromUserInput(const QString& string);

public slots:
    MainWindow* newWindow(const QString &url = QString());

private:
    void buildUI();

private:
    MainView* m_view;
    QGraphicsScene* m_scene;
    WebView* m_webViewItem;
    WebPage* m_page;
    QNetworkProxy* m_proxy;     // not owned (FIXME)
    Settings m_settings;

    QLineEdit* urlEdit;
};



class WebView : public QGraphicsWebView {
    Q_OBJECT

public:
    WebView(QGraphicsItem* parent = 0)
        : QGraphicsWebView(parent)
    {
    }
};

class WebPage : public QWebPage {
    Q_OBJECT

public:
    WebPage(QObject* parent, MainWindow* ownerWindow)
        : QWebPage(parent)
        , m_ownerWindow(ownerWindow)
        {}

    virtual QWebPage* createWindow(QWebPage::WebWindowType);

private:
    MainWindow* m_ownerWindow;
};



#endif
