#ifndef PannableTileContainer_h_
#define PannableTileContainer_h_

#include "PannableViewport.h"
#include "CommonGestureRecognizer.h"

class HomeView;
class QGraphicsSceneMouseEvent;

class PannableTileContainer : public PannableViewport, private CommonGestureConsumer {
    Q_OBJECT
public:
    PannableTileContainer(QGraphicsItem*, Qt::WindowFlags wFlags = 0);
    ~PannableTileContainer();
    
    void cancelLeftMouseButtonPress(const QPoint&);

protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);

    void mousePressEventFromChild(QGraphicsSceneMouseEvent*);
    void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent*);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent*);
    void adjustClickPosition(QPointF&) {}

private:
    void forwardEvent(QGraphicsSceneMouseEvent*);

private:
    CommonGestureRecognizer m_recognizer;
    QGraphicsSceneMouseEvent* m_selfSentEvent;
};

#endif
