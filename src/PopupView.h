#ifndef PopupView_h_
#define PopupView_h_

#include "TileSelectionViewBase.h"

class PannableTileContainer;
class PopupWidget;
class TileItem;

class PopupView : public TileSelectionViewBase {
    Q_OBJECT
public:
    PopupView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~PopupView();

    void setGeometry(const QRectF& rect);

    void setFilterText(const QString& text);

Q_SIGNALS:
    void pageSelected(const QUrl&);

private Q_SLOTS:
    void tileItemActivated(TileItem*);

private:
    bool setupInAndOutAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    PopupWidget* m_popupWidget;
    PannableTileContainer* m_pannableContainer;
    QString m_filterText;
};

#endif
