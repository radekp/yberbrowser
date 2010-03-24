#ifndef WebViewport_h_
#define WebViewport_h_

#include "yberconfig.h"
#if USE_DUI
#error should not be used in DUI
#endif
#include <QGraphicsItemAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QTimer>

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"

class WebViewportItem;

class WebViewport : public PannableViewport, private CommonGestureConsumer
{
    Q_OBJECT
public:
    WebViewport(WebViewportItem* viewportWidget, QGraphicsItem* parent = 0);

    WebViewportItem* viewportWidget() const;

    void startZoomAnimToItemHotspot(const QPointF& hotspot,  const QPointF& viewTargetHotspot, qreal scale);

public Q_SLOTS:
    void reset();

protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);
    void wheelEvent(QGraphicsSceneWheelEvent*);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);

    void cancelLeftMouseButtonPress(const QPoint&);

    void mousePressEventFromChild(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent * event);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent * event);

protected Q_SLOTS:
    void contentsSizeChangeCausedResize();

private:
    void resetZoomAnim();
    void wheelEventFromChild(QGraphicsSceneWheelEvent *event);
    bool mouseEventFromChild(QGraphicsSceneMouseEvent *event);
    bool isZoomedIn() const;

    CommonGestureRecognizer m_recognizer;
    QEvent* m_selfSentEvent;

    QTimer m_doubleClickWaitTimer;
};


#endif
