#ifndef PopupView_h_
#define PopupView_h_

#include "TileSelectionViewBase.h"

class PannableTileContainer;
class PopupWidget;
class TileItem;
class Suggest;
class QTimer;

class PopupView : public TileSelectionViewBase {
    Q_OBJECT
public:
    PopupView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~PopupView();

    void resizeEvent(QGraphicsSceneResizeEvent* event);

    void setFilterText(const QString& text);

Q_SIGNALS:
    void pageSelected(const QUrl&);

private Q_SLOTS:
    void tileItemActivated(TileItem*);
    void startSuggest();
    void populateSuggestion();

private:
    bool setupInAndOutAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    Suggest* m_suggest;
    QTimer* m_suggestTimer;
    PopupWidget* m_popupWidget;
    PannableTileContainer* m_pannableContainer;
    QString m_filterText;
};

#endif
