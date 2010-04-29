#ifndef TabSelectionView_h_
#define TabSelectionView_h_

#include "TileSelectionViewBase.h"
#include "UrlItem.h"

class PannableTileContainer;
class TabWidget;
class WebView;
class UrlItem;

class TabSelectionView :  public TileSelectionViewBase {
    Q_OBJECT
public:
    TabSelectionView(QList<WebView*>& windowList, WebView* activeWindow, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~TabSelectionView();

    void setGeometry(const QRectF& rect);

Q_SIGNALS:
    void windowSelected(WebView* webView);
    void createNewWindow();

public Q_SLOTS:
    void tileItemActivated(UrlItem*);

private:
    void setupAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    PannableTileContainer* m_pannableTabContainer;
    TabWidget* m_tabWidget;
    QList<WebView*>* m_windowList;
    UrlList m_tabList;
};

#endif
