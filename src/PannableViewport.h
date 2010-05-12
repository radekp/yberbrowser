#ifndef PannableViewport_h
#define PannableViewport_h

#include "yberconfig.h"

#if USE_DUI

#include <DuiPannableViewport>
class PannableViewport : public DuiPannableViewport
{
    Q_OBJECT
};

#else

#include <QGraphicsWidget>
#include <QPropertyAnimation>

#include "3rdparty/qabstractkineticscroller.h"

class ScrollbarItem;

class PannableViewport : public QGraphicsWidget, public YberHack_Qt::QAbstractKineticScroller
{
    Q_OBJECT
    Q_PROPERTY(QPointF panPos READ panPos WRITE setPanPos);
public:

    PannableViewport(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~PannableViewport();

    void setPanPos(const QPointF& pos);
    QPointF panPos() const;

    void setRange(const QRectF&);
    void setAutoRange(bool) { }

    void updateScrollbars();

    void setWidget(QGraphicsWidget*);
    void detachWidget() { m_pannedWidget = 0; }

protected:
    bool sceneEvent(QEvent* e);
    bool sceneEventFilter(QGraphicsItem *i, QEvent *e);

    QSize viewportSize() const;
    QPoint maximumScrollPosition() const;
    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint &pos, const QPoint &overShootDelta);

    virtual void stateChanged(YberHack_Qt::QAbstractKineticScroller::State oldState, YberHack_Qt::QAbstractKineticScroller::State newState);
    bool canStartScrollingAt(const QPoint &globalPos) const;
    QPointF clipPointToViewport(const QPointF& p) const;

    QGraphicsWidget* pannedWidget() const { return m_pannedWidget; }
    virtual void setPannedWidgetGeometry(const QRectF& r);

    void startPannedWidgetGeomAnim(const QRectF& geom);
    void stopPannedWidgetGeomAnim();

protected Q_SLOTS:
    void geomAnimStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State);

private:
    void transferAnimStateToView();
    QRectF adjustRectForPannedWidgetGeometry(const QRectF&);

    QGraphicsWidget* m_pannedWidget;
    ScrollbarItem* m_vScrollbar;
    ScrollbarItem* m_hScrollbar;
    QPointF m_overShootDelta;
    QPointF m_extraPos;
    QRectF m_geomAnimEndValue;
    QPropertyAnimation m_geomAnim;
};

#endif
#endif
