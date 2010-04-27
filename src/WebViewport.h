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

#define ENABLE_LINK_SELECTION_VISUAL_DEBUG

class WebViewportItem;
class LinkSelectionItem;
class QGraphicsSceneMouseEvent;
#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
class QGraphicsRectItem;
class QGraphicsEllipseItem;
#endif

class WebViewport : public PannableViewport, private CommonGestureConsumer
{
    Q_OBJECT
public:
    WebViewport(WebViewportItem* viewportWidget, QGraphicsItem* parent = 0);
    ~WebViewport();

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
    void adjustClickPosition(QPointF& pos);
    void setPannedWidgetGeometry(const QRectF& r);

protected Q_SLOTS:
    void contentsSizeChangeCausedResize();
    void startLinkSelection();

private:
    void resetZoomAnim();
    void wheelEventFromChild(QGraphicsSceneWheelEvent *event);
    bool mouseEventFromChild(QGraphicsSceneMouseEvent *event);
    bool isZoomedIn() const;

    CommonGestureRecognizer m_recognizer;
    QEvent* m_selfSentEvent;

    QTimer m_doubleClickWaitTimer;
    LinkSelectionItem* m_linkSelectionItem;
    QGraphicsSceneMouseEvent* m_delayedMouseReleaseEvent;
#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
    QGraphicsRectItem* m_searchRectItem;
    QGraphicsEllipseItem* m_clickablePointItem;
#endif
};


#endif
