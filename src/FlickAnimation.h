#ifndef FlickAnimation_h_
#define FlickAnimation_h_

#include <QObject>
#include <QTime>

class WebViewportItem;

class FlickAnimation : public QObject
{
    Q_OBJECT

public:

    FlickAnimation(WebViewportItem* item);
    ~FlickAnimation();

    void start(qreal velocity);
    void stop();

protected:
    void timerEvent(QTimerEvent *event);
private:
    void stopTimer();

private:
    WebViewportItem* m_item;
    int m_timerId;
    QTime m_velocityTs;
    qreal m_currentVelocity;
};




#endif
