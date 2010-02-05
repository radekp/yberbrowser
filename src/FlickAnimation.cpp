#include <QtGui>
#include "FlickAnimation.h"
#include "WebViewportItem.h"

// timer resolution in milliseconds
static const int s_timerRes = 1000. / 60.;

// acceleration / deceleration in pixels per millisecond^2
static const float s_acceleration = 1./300.;
static const float s_maxVelocity = 2.8;

FlickAnimation::FlickAnimation(WebViewportItem* item)
    : m_item(item)
    , m_timerId(0)
{

}

FlickAnimation::~FlickAnimation()
{
    stopTimer();
}

void FlickAnimation::start(qreal velocity)
{
    stopTimer();

    m_velocityTs.start();
    m_currentVelocity = velocity;
    // max it out at +-1.9
    if (qAbs(m_currentVelocity) > s_maxVelocity)
        m_currentVelocity = s_maxVelocity * (m_currentVelocity < 0 ? -1 : 1);

    m_timerId = startTimer(s_timerRes);

}

void FlickAnimation::stop()
{
    stopTimer();
}


void FlickAnimation::timerEvent(QTimerEvent */*event*/)
{
    int dt = m_velocityTs.restart();

    float ay = -s_acceleration;
    if (m_currentVelocity < 0)
        ay =  s_acceleration;

    // y(t) = v0*t + (1/2)*a*t^2
    // v(t) = v0 + at
    int dy = 0;

    float vy = m_currentVelocity;
    float vy_1 = vy + ay*dt;

    if ((vy <= 0 && vy_1 > 0) || (vy >= 0 && vy_1 < 0)) {
        // Sign change means velocity ran out. Calculate the position
        // in time when the velocity was zero
        // v(t) = 0 --> t = -v0/a

        float ddt = -vy/ay;
        dy = vy*ddt + 0.5*ay*ddt*ddt;

        vy_1 = 0.;
        stopTimer();
    } else if (vy != 0){
        dy = vy*dt + 0.5*ay*dt*dt;
    }
    m_currentVelocity = vy_1;

    m_item->moveItemBy(QPointF(0, dy));
}

void FlickAnimation::stopTimer()
{
    if (m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

