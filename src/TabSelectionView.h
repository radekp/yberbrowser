#ifndef TabSelectionView_h_
#define TabSelectionView_h_

#include "TileSelectionViewBase.h"
#include "UrlItem.h"

class PannableTileContainer;
class TabWidget;
class WebView;
class TileItem;

class TabSelectionView :  public TileSelectionViewBase {
    Q_OBJECT
public:
    TabSelectionView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~TabSelectionView();

    void setWindowList(QList<WebView*>& windowList, WebView& activeWindow) { m_windowList = &windowList; m_activeWindow = &activeWindow; }
    void setGeometry(const QRectF& rect);

Q_SIGNALS:
    void windowSelected(WebView* webView);
    void windowClosed(WebView* webView);
    void windowCreated();

public Q_SLOTS:
    void tileItemActivated(TileItem*);
    void tileItemClosed(TileItem*);

private:
    bool setupAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    PannableTileContainer* m_pannableTabContainer;
    TabWidget* m_tabWidget;
    QList<WebView*>* m_windowList;
    WebView* m_activeWindow;
};

#endif
