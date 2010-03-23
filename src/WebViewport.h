#ifndef WebViewport_h_
#define WebViewport_h_

#include "yberconfig.h"
#include <QGraphicsItemAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QPropertyAnimation>
#include <QTimeLine>

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"

class WebViewportItem;

class WebViewport : public PannableViewport, private CommonGestureConsumer
{
public:
    WebViewport(QGraphicsItem* parent = 0);

    WebViewportItem* viewportItem() const;

    void startZoomAnimToItemHotspot(const QPointF& hotspot, qreal scale);

protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);
    void wheelEvent(QGraphicsSceneWheelEvent*);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);

    void cancelLeftMouseButtonPress(const QPoint&);

    void tapGesture(QGraphicsSceneMouseEvent* , QGraphicsSceneMouseEvent* ) {}
    void doubleTapGesture(QGraphicsSceneMouseEvent* ) {}
    void touchGestureBegin(const QPointF&) {}
    void touchGestureEnd() {}

protected Q_SLOTS:
    void commitZoom();
    void zoomAnimStateChanged(QTimeLine::State newState);

private:
    void sendPendingMouseClick();
    void resetZoomAnim();
    void wheelEventFromChild(QGraphicsSceneWheelEvent *event);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent * event);
    bool mouseEventFromChild(QGraphicsSceneMouseEvent *event);

    void transferAnimStateToView();
    bool isZoomedIn() const;

    CommonGestureRecognizer m_recognizer;
    QGraphicsSceneMouseEvent m_pendingPress;
    QGraphicsSceneMouseEvent m_pendingRelease;
    bool m_pendingPressValid;

    qreal m_zoomScale;
    QGraphicsItemAnimation m_zoomAnim;
};


#endif
