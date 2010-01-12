#ifndef CommonGestureRecognizer_h_
#define CommonGestureRecognizer_h_

#include <QBasicTimer>
#include <QObject>
#include <QPointF>
#include <QTime>

class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class QPointF;

class CommonGestureConsumer
{

    // interface "TapGestureConsumer"
public:
    virtual void tapGesture(QGraphicsSceneMouseEvent* pressEventLike, QGraphicsSceneMouseEvent* releaseEventLike) = 0;
    virtual void doubleTapGesture(QGraphicsSceneMouseEvent* pressEventLike) = 0;


    // interface "PanGestureConsumer"
public:
    enum PanDirection {
        HPan,
        VPan
    };
    virtual bool isPanning() const = 0;
    virtual void startPanGesture(CommonGestureConsumer::PanDirection) = 0;
    virtual void panBy(const QPointF& delta) = 0;
    virtual void stopPanGesture() = 0;

public:
    // interaface "FlickGestureConsumer"
    virtual void flickGesture(qreal velocityX, qreal velocityY) = 0;

};


class CommonGestureRecognizer : public QObject
{
    Q_OBJECT

public:
    CommonGestureRecognizer(CommonGestureConsumer*);
    ~CommonGestureRecognizer();

    bool filterMouseEvent(QGraphicsSceneMouseEvent *);

    void reset();
    
protected:
    void timerEvent(QTimerEvent *event);

private:
    void clearDelayedPress();
    void capturePressOrRelease(QGraphicsSceneMouseEvent *event, bool wasRelease = false);
    void sendDelayedPress();

    bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    bool mousePressEvent(QGraphicsSceneMouseEvent* event);
    bool mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    bool mouseMoveEvent(QGraphicsSceneMouseEvent* event);


private:
    CommonGestureConsumer* m_consumer;

    QGraphicsSceneMouseEvent* m_delayedPressEvent;
    QGraphicsSceneMouseEvent* m_delayedReleaseEvent;
    QTime m_delayedPressMoment;
    QTime m_doubleClickFilter;
    QTime m_panVelocitySamplingTs;
    QBasicTimer m_delayedPressTimer;
    int m_pressDelay;
    QPointF m_panVelocity;
    QPointF m_dragStartPos;
};

#endif
