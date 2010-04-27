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

public:
    virtual void mousePressEventFromChild(QGraphicsSceneMouseEvent*) = 0;
    virtual void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent* ) = 0;
    virtual void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent*) = 0;
    virtual void adjustClickPosition(QPointF& pos) = 0;
};

class CommonGestureRecognizer : public QObject
{
    Q_OBJECT

public:
    CommonGestureRecognizer(CommonGestureConsumer*);
    ~CommonGestureRecognizer();

    bool filterMouseEvent(QGraphicsSceneMouseEvent *);

    void reset();
    void clearDelayedPress();

protected:
    void timerEvent(QTimerEvent *event);

private:
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
