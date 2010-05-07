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
    TileSelectionViewBase(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TileSelectionViewBase();

    void setGeometry(const QRectF& rect);

    bool isActive() const { return m_active; }

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
    void startAnimation(bool);

    virtual void createViewItems() = 0;
    virtual void destroyViewItems() = 0;
    virtual bool setupAnimation(bool) = 0;
    virtual void connectItem(TileItem&);

protected:
    QParallelAnimationGroup* m_slideAnimationGroup;

private:
    QGraphicsRectItem* m_bckg;
    bool m_active;
};
#endif
