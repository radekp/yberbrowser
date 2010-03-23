#ifndef WebViewport_h_
#define WebViewport_h_

#include "yberconfig.h"
#include <QGraphicsItemAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QPropertyAnimation>
#include <QTimeLine>
#include <QTimer>

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"

class WebViewportItem;

class WebViewport : public PannableViewport, private CommonGestureConsumer
{
    Q_OBJECT
public:
    WebViewport(QGraphicsItem* parent = 0);

    WebViewportItem* viewportItem() const;

    void startZoomAnimToItemHotspot(const QPointF& hotspot, qreal scale);

protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);
    void wheelEvent(QGraphicsSceneWheelEvent*);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);

    void cancelLeftMouseButtonPress(const QPoint&);

    void mousePressEventFromChild(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent * event);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent * event);

protected Q_SLOTS:
    void zoomAnimStateChanged(QTimeLine::State newState);

private:

    void resetZoomAnim();
    void wheelEventFromChild(QGraphicsSceneWheelEvent *event);
    bool mouseEventFromChild(QGraphicsSceneMouseEvent *event);

    void transferAnimStateToView();
    bool isZoomedIn() const;
    void startPendingMouseClickTimer();

    CommonGestureRecognizer m_recognizer;
    QEvent* m_selfSentEvent;
    bool m_pendingPressValid;

    qreal m_zoomScale;
    QGraphicsItemAnimation m_zoomAnim;
    QTimer m_doubleClickWaitTimer;

};


#endif
