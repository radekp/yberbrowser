#ifndef TabSelectionView_h_
#define TabSelectionView_h_

#include "TileSelectionViewBase.h"

class PannableTileContainer;
class TabWidget;
class UrlItem;

class TabSelectionView :  public TileSelectionViewBase {
    Q_OBJECT
public:
    TabSelectionView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~TabSelectionView();

    void setGeometry(const QRectF& rect);

Q_SIGNALS:
    void tabSelected();

public Q_SLOTS:
    void tileItemActivated(UrlItem*);

private:
    void setupAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    PannableTileContainer* m_pannableTabContainer;
    TabWidget* m_tabWidget;
};

#endif
