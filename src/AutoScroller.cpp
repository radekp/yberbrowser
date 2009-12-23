#include <QDebug>
#include "AutoScroller.h"

void AutoScroller::install(QGraphicsView* view, QGraphicsWidget* scrolledWidget)
{
    m_scrolledWidget = scrolledWidget;
    m_view = view;

    connect(&m_scrollTimer, SIGNAL(timeout()), this, SLOT(advanceScroll()));
    m_scrollTimer.setInterval(15);
    m_scrollTimer.start();
    m_timeLine.start();
}

void AutoScroller::advanceScroll()
{
    qDebug() << "Advance";
    int max = m_view->verticalScrollBar()->maximum();
    if (!max)
        return;

    int elapsed = m_timeLine.elapsed();
    int localTimeMax = 1000 * max / 120;
    int localTime = elapsed % localTimeMax;
    double progress = (double)(localTime) / localTimeMax;
    int scrollPosition = progress * max;
    m_scrolledWidget->setY(-scrollPosition);
}
