#include "AutoScrollTest.h"
#include "BrowsingView.h"
#include "ApplicationWindow.h"
#include "PannableViewport.h"
#include "WebViewport.h"
#include "WebViewportItem.h"
#include "WebView.h"

#include <QGraphicsWidget>
#include <QPainterPath>

namespace {
const int s_scrollPixels[] = {10, 20, 30, 40, 50, 10};
const int s_scrollPixelsTimeot[] = {3000, 3000, 4000, 4000, 3000, 2000};
const int s_xAxisInFPS[] = {10, 20, 60};
const QColor s_xFPSValueColor(10, 255, 10);
const QColor s_FPSTextColor(10, 255, 10);
const QColor s_FPSDividerColor(255, 10, 10);
const QColor s_FPSLineColor(10, 255, 10);
const QColor s_bckColor(20, 20, 20);
const qreal s_bckTransparency = 0.9;
const int s_fpsCheckTimeout = 100;
const int s_scrollTimeout = 10;
}

class FPSResultView : public QGraphicsWidget {
    Q_OBJECT
public:
    FPSResultView(QList<int>&, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~FPSResultView();

    void appear(ApplicationWindow*);

Q_SIGNALS:
    void finished();

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
    void addAreaDividerItem(int);
    void addHorizontalLineItem(int);
    void addTextItem(const QRectF&, const QString&);
    void addTransparentRectangleItem(const QRectF&);
    void displayResult();
    void startAnimation(bool);
    qreal getYValue(qreal fps);

private Q_SLOTS:
    void animFinished();

private:
    QList<int>* m_fpsValues;
    QRectF m_rect;
    int m_min;
    int m_max;
    int m_avg;
};

FPSResultView::FPSResultView(QList<int>& fpsValues, QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_fpsValues(&fpsValues)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
}

FPSResultView::~FPSResultView()
{
}

void FPSResultView::appear(ApplicationWindow *window)
{
    if (!scene()) {
        window->scene()->addItem(this);
        setZValue(100);
    }
    scene()->setActiveWindow(this);

    startAnimation(true);
}

qreal FPSResultView::getYValue(qreal fps) 
{
    // squeeze it to the middle 
    qreal ed = m_rect.height() / 2 / (m_max - m_min);
    return (m_rect.bottom() - (m_rect.height() / 4) - (ed * (fps - m_min)));
}

void FPSResultView::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    startAnimation(false);
}

void FPSResultView::addAreaDividerItem(int x)
{
    QGraphicsLineItem* areaDivider = new QGraphicsLineItem(x, m_rect.top(), x, m_rect.bottom(), this);
    areaDivider->setPen(QPen(QBrush(s_FPSDividerColor), 1, Qt::DashLine));
}

void FPSResultView::addHorizontalLineItem(int y)
{
    QGraphicsLineItem* lineItem = new QGraphicsLineItem(m_rect.left(), y, m_rect.right(), y, this);
    lineItem->setPen(QPen(QBrush(s_xFPSValueColor), 1, Qt::DotLine));
}

void FPSResultView::addTextItem(const QRectF& rect, const QString& text)
{
    QFont f("Times", 10);
    QFontMetrics m(f);

    QGraphicsSimpleTextItem* fpsTextItem = new QGraphicsSimpleTextItem(text, this);
    fpsTextItem->setFont(f);
    fpsTextItem->setPos(rect.left() + rect.width()/2 - m.width(text)/2, rect.top() + rect.height() / 2 - m.height() / 2);
    fpsTextItem->setPen(s_FPSTextColor);
    fpsTextItem->setBrush(s_FPSTextColor);
}

void FPSResultView::addTransparentRectangleItem(const QRectF& rect)
{
    QGraphicsRectItem* bckg = new QGraphicsRectItem(rect, this);
    bckg->setBrush(s_bckColor);
    bckg->setOpacity(s_bckTransparency);
}

void FPSResultView::displayResult()
{
    if (!m_fpsValues->size())
        return;

    // background
    m_rect = rect();
    int xPadding = m_rect.width() / 10;
    int yPadding = m_rect.height() / 10;
    m_rect.adjust(xPadding, yPadding, -xPadding, -yPadding);
    addTransparentRectangleItem(m_rect);

    // calculate min max and avg first to setup the grid
    m_min = m_fpsValues->at(0);
    m_max = m_min;
    m_avg = m_fpsValues->at(0);
    int count = 1;
    for (int i = 1; i < m_fpsValues->size(); ++i) {
        // reserved value
        if (m_fpsValues->at(i) == -1)
            continue;
        m_max = qMax(m_fpsValues->at(i), m_max);
        m_min = qMin(m_fpsValues->at(i), m_min);
        m_avg+=m_fpsValues->at(i);
        count++;
    }
    m_avg/=count++;

    // create path for fps
    QPainterPath path;

    qreal ed = m_rect.width() / m_fpsValues->size();
    qreal x = m_rect.left(); 
    qreal y = getYValue(m_fpsValues->at(0));
    path.moveTo(x, y);
    // add fps items and scroll divider
    for (int i = 1; i < m_fpsValues->size(); ++i) {
        x = m_rect.left() + i*ed;

        // -1 indicates new scroll sections (up, down, up, down)
        if (m_fpsValues->at(i) == -1) {
            addAreaDividerItem(x);
            continue;
        }
        // add fps value to the path
        y = getYValue(m_fpsValues->at(i));
        path.lineTo(x, y);
    }
    
    QGraphicsPathItem* results = new QGraphicsPathItem(path, this);
    results->setPen(QPen(QBrush(s_FPSLineColor), 1));

    // left side rectangle for fps text
    QRectF sideRect(m_rect); sideRect.moveLeft(sideRect.left() - 30); sideRect.setWidth(20);
    addTransparentRectangleItem(sideRect);
    for (unsigned int i = 0; i < sizeof(s_xAxisInFPS) / sizeof(int); ++i) {
        y = getYValue(s_xAxisInFPS[i]);
        // out of bounds? could happen on non target env, when fps average is so high
        if (!m_rect.contains(m_rect.left(), y))
            continue;
        addHorizontalLineItem(y);
        addTextItem(QRectF(sideRect.left(), y - 10, sideRect.width(), 20), QString::number(s_xAxisInFPS[i]));
    }    

    // add avg as an fps horizontal line too
    addHorizontalLineItem(getYValue(m_avg));
    addTextItem(QRectF(sideRect.left(), getYValue(m_avg) - 10, sideRect.width(), 20), QString::number(m_avg));

    // top rectangle item with min, max and avg
    QRectF topRect(m_rect); topRect.moveTop(sideRect.top() - 30); topRect.setHeight(20);
    addTransparentRectangleItem(topRect);
    addTextItem(QRect(topRect.left() + 5, topRect.top(), 50, topRect.height()), QString("Min:" + QString::number(m_min) + "fps"));
    addTextItem(QRect(topRect.left() + 100, topRect.top(), 50, topRect.height()), QString("Max:" + QString::number(m_max) + "fps"));
    addTextItem(QRect(topRect.left() + 200, topRect.top(), 50, topRect.height()), QString("Avg:" + QString::number(m_avg) + "fps"));
}

void FPSResultView::startAnimation(bool in)
{
    QPropertyAnimation* animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(1000);

    QRectF r(rect());
    QRectF hidden(r.x(), r.bottom() + r.height(), r.width(), r.height());

    animation->setStartValue(in ?  hidden : r);
    animation->setEndValue(in ? r : hidden);

    animation->setEasingCurve(in ? QEasingCurve::OutBack : QEasingCurve::OutCubic);
    animation->start();

    if (in) {
        setGeometry(hidden);
        displayResult();
    } else
        connect(animation, SIGNAL(finished()), this, SLOT(animFinished()));
}

void FPSResultView::animFinished()
{
    emit finished();
}

AutoScrollTest::AutoScrollTest(BrowsingView* browsingView)
    : m_browsingView(browsingView)
    , m_scrollTimer(this)
    , m_fpsResultView(0)
{
    connect(&m_scrollTimer, SIGNAL(timeout()), this, SLOT(doScroll()));
    connect(&m_fpsTimer, SIGNAL(timeout()), this, SLOT(fpsTick()));
}

AutoScrollTest::~AutoScrollTest()
{
    delete m_fpsResultView;
}

void AutoScrollTest::starScrollTest()
{
    if (!m_browsingView)
        return;

    // load news.google.com if nothing is loaded.
    QGraphicsWebView* webView = ((WebViewport*)m_browsingView->pannableViewport())->viewportWidget()->webView();
    if (webView->url().isEmpty()) {
        m_browsingView->load(QUrl("http://news.google.com"));
        connect(webView->page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        return;
    }

    m_scrollIndex = 0;
    m_fpsTicks = ((WebView*)((WebViewport*)m_browsingView->pannableViewport())->viewportWidget()->webView())->fpsTicks();
    m_fpsTimestamp.start();
    m_fpsTimer.start(s_fpsCheckTimeout);
    m_scrollTimer.start(s_scrollTimeout);   
    m_scrollValue = s_scrollPixels[0];
    QTimer::singleShot(s_scrollPixelsTimeot[0], this, SLOT(scrollTimeout()));
}

void AutoScrollTest::doScroll()
{
    PannableViewport* viewport = m_browsingView->pannableViewport();
    QPointF panPos(viewport->panPos().x(), viewport->panPos().y() - m_scrollValue);
    viewport->setPanPos(panPos);

    // switch direction
    if (panPos.y() != viewport->panPos().y())
        m_scrollValue = -m_scrollValue;
}

void AutoScrollTest::fpsTick()
{
    int prevfps = m_fpsTicks;
    m_fpsTicks = ((WebView*)((WebViewport*)m_browsingView->pannableViewport())->viewportWidget()->webView())->fpsTicks();

    double dt = m_fpsTimestamp.restart();
    double dticks = m_fpsTicks - prevfps;
    if (dt)
        m_fpsValues.append((dticks *  1000.) / dt);
}

void AutoScrollTest::loadFinished(bool success)
{
    // dont start scrolling right after page is loaded. it alters the result
    if (success)
        QTimer::singleShot(1000, this, SLOT(starScrollTest()));
        
}

void AutoScrollTest::scrollTimeout()
{
    m_fpsValues.append(-1);
    // finished?
    if (++m_scrollIndex < sizeof(s_scrollPixelsTimeot) / sizeof(int)) {
        m_scrollValue = s_scrollPixels[m_scrollIndex < sizeof(s_scrollPixels) / sizeof(int) ? m_scrollIndex : 0];
        QTimer::singleShot(s_scrollPixelsTimeot[m_scrollIndex], this, SLOT(scrollTimeout()));
    } else {
        m_scrollTimer.stop();
        m_fpsTimer.stop();
        //
        m_fpsResultView = new FPSResultView(m_fpsValues);
        m_fpsResultView->setGeometry(m_browsingView->pannableViewport()->rect());
        m_fpsResultView->appear(m_browsingView->applicationWindow());
        connect(m_fpsResultView, SIGNAL(finished()), this, SLOT(fpsViewClicked()));
    }

}

void AutoScrollTest::fpsViewClicked()
{
    emit finished();
}

#include "AutoScrollTest.moc"
