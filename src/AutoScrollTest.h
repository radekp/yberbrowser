#ifndef AutoScrollTest_h
#define AutoScrollTest_h

#include "yberconfig.h"
#include <QTime>
#include <QTimer>
#include <QPoint>

class BrowsingView;
class FPSResultView;
class QGraphicsSceneMouseEvent;

class AutoScrollTest : public QObject
{
    Q_OBJECT
public:
    AutoScrollTest(BrowsingView*);
    ~AutoScrollTest();

public Q_SLOTS:
    void starScrollTest();
    void doScroll();
    void fpsTick();
    void loadFinished(bool);
    void scrollTimeout();

Q_SIGNALS:
    void finished();

private Q_SLOTS:
    void fpsViewClicked();

private:
    BrowsingView* m_browsingView;
    QTimer m_scrollTimer;
    unsigned int m_scrollIndex;
    int m_scrollValue;
    QTime m_fpsTimestamp;
    QTimer m_fpsTimer;
    unsigned int m_fpsTicks;
    QList<int> m_fpsValues;
    FPSResultView* m_fpsResultView;
};
#endif
