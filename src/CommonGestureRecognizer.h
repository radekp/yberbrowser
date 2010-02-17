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

    //interface "TouchGestureConsumer"
public:
    virtual void touchGestureBegin(const QPointF&) = 0;
    virtual void touchGestureEnd() = 0;

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
    QBasicTimer m_delayedPressTimer;
    int m_pressDelay;
};

#endif
