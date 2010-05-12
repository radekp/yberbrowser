#ifndef TileSelectionViewBase_h_
#define TileSelectionViewBase_h_

#include "TileContainerWidget.h"

class QGraphicsRectItem;
class ApplicationWindow;
class QParallelAnimationGroup;
class TileItem;

class TileSelectionViewBase : public QGraphicsWidget {
    Q_OBJECT
public:
    enum ViewType {
        Home,
        UrlPopup
    };

    virtual ~TileSelectionViewBase();

    void resizeEvent(QGraphicsSceneResizeEvent* event);

    bool active() const { return m_active; }
    ViewType viewtype() const { return m_type; }

Q_SIGNALS:
    void appeared();
    void disappeared();

public Q_SLOTS:
    void appear(ApplicationWindow*);
    void disappear();
    void animFinished();
    virtual void tileItemActivated(TileItem*);
    virtual void tileItemClosed(TileItem*) {}
    virtual void tileItemEditingMode(TileItem*) {}

protected:
    TileSelectionViewBase(ViewType type, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);

    virtual void startInOutAnimation(bool);

    virtual void createViewItems() = 0;
    virtual void destroyViewItems() = 0;
    virtual void connectItem(TileItem&);

protected:
    QParallelAnimationGroup* m_slideAnimationGroup;

private:
    QGraphicsRectItem* m_bckg;
    bool m_active;
    ViewType m_type;
};
#endif
