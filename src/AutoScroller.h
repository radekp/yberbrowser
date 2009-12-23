#ifndef AutoScroller_h_
#define AutoScroller_h_

#include <QtGui>
#include <QGraphicsView>
#include <QGraphicsWidget>

class AutoScroller : QObject {
    Q_OBJECT

public:
    ~AutoScroller() {}
    void install(QGraphicsView*, QGraphicsWidget*);

protected slots:
    void advanceScroll();

private:
    QGraphicsView* m_view;
    QGraphicsWidget* m_scrolledWidget;
    QTimer m_scrollTimer;
    QTime m_timeLine;
};


#endif
