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
const int s_scrollPixels[] = {10, -20, 30, -40, 50, -60};
const int s_xAxisInFPS[] = {10, 20, 60};
const QColor s_xFPSValueColor(10, 255, 10);
const QColor s_FPSTextColor(10, 255, 10);
const QColor s_FPSDividerColor(255, 10, 10);
const QColor s_FPSLineColor(10, 255, 10);
const QColor s_bckColor(20, 20, 20);
const qreal s_bckTransparency = 0.9;
const int s_fpsCheckTimeout = 100;
const int s_scrolTimeout = 0;
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
    void addAreaDividerItem(int, int, int);
    void addLineItem(int, int, int);
    void addTextItem(const QRectF&, const QString&);
    void addTransparentRectangleItem(const QRectF&);
    void displayResult();
    void startAnimation(bool);

private Q_SLOTS:
    void animFinished();

private:
    QList<int>* m_fpsValues;
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

qreal getYValue(QRectF rect, qreal fps) 
{
    return qMin(rect.bottom(), qMax(rect.top(), rect.bottom() - fps * 3));
}

void FPSResultView::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
    startAnimation(false);
}

void FPSResultView::addAreaDividerItem(int x, int y1, int y2)
{
    QGraphicsLineItem* areaDivider = new QGraphicsLineItem(x, y1, x, y2, this);
    areaDivider->setPen(QPen(QBrush(s_FPSDividerColor), 1, Qt::DashLine));
}

void FPSResultView::addLineItem(int x1, int x2, int y)
{
    QGraphicsLineItem* lineItem = new QGraphicsLineItem(x1, y, x2, y, this);
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
    QRectF r(rect());
    int xPadding = r.width() / 10;
    int yPadding = r.height() / 10;
    r.adjust(xPadding, yPadding, -xPadding, -yPadding);
    addTransparentRectangleItem(r);

    // left side rectangle for fps text
    QRectF sideRect(r); sideRect.moveLeft(sideRect.left() - 30); sideRect.setWidth(20);
    addTransparentRectangleItem(sideRect);
    for (unsigned int i = 0; i < sizeof(s_xAxisInFPS) / sizeof(int); ++i) {
        qreal fpsValue = getYValue(r, s_xAxisInFPS[i]);

        addLineItem(r.left(), r.right(), fpsValue);
        addTextItem(QRectF(sideRect.left(), fpsValue - 10, sideRect.width(), 20), QString::number(s_xAxisInFPS[i]));
    }    

    // create path for fps
    QPainterPath path;

    qreal ed = r.width() / m_fpsValues->size();
    qreal x = r.left(); 
    qreal y = getYValue(r, m_fpsValues->at(0));
    path.moveTo(x, y);
    int min = m_fpsValues->at(0);
    int max = min;
    int avg = m_fpsValues->at(0);
    // add fps items and scroll divider
    for (int i = 1; i < m_fpsValues->size(); ++i) {
        x = r.left() + i*ed;

        // -1 indicates new scroll sections (up, down, up, down)
        if (m_fpsValues->at(i) == -1) {
            addAreaDividerItem(x, r.top(), r.bottom());
            continue;
        }
        // add fps value to the path
        y = getYValue(r, m_fpsValues->at(i));
        path.lineTo(x, y);
        max = qMax(m_fpsValues->at(i), max);
        min = qMin(m_fpsValues->at(i), min);
        avg+=m_fpsValues->at(i);
    }
    avg/=m_fpsValues->size();
    
    QGraphicsPathItem* results = new QGraphicsPathItem(path, this);
    results->setPen(QPen(QBrush(s_FPSLineColor), 1));

    // top rectangle item with min, max and avg
    QRectF topRect(r); topRect.moveTop(sideRect.top() - 30); topRect.setHeight(20);
    addTransparentRectangleItem(topRect);
    addTextItem(QRect(topRect.left() + 5, topRect.top(), 50, topRect.height()), QString("Min:" + QString::number(min) + "fps"));
    addTextItem(QRect(topRect.left() + 100, topRect.top(), 50, topRect.height()), QString("Max:" + QString::number(max) + "fps"));
    addTextItem(QRect(topRect.left() + 200, topRect.top(), 50, topRect.height()), QString("Avg:" + QString::number(avg) + "fps"));

    // add avg as an fps horizontal line too
    addLineItem(r.left(), r.right(), getYValue(r, avg));
    addTextItem(QRectF(sideRect.left(), getYValue(r, avg) - 10, sideRect.width(), 20), QString::number(avg));
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
    connect(&m_scrollTimer, SIGNAL(timeout()), this, SLOT(scroll()));
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

    m_browsingView->hideHistoryView();

    // load news.google.com if nothing is loaded.
    QGraphicsWebView* webView = ((WebViewport*)m_browsingView->pannableViewport())->viewportWidget()->webView();
    if (webView->url().isEmpty()) {
        m_browsingView->load(QUrl("http://news.google.com"));
        connect(webView->page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        return;
    }

    m_lastPos = QPoint(0, 0);
    m_scrollIndex = 0;
    m_fpsTicks = 0;
    m_fpsTimestamp.start();
    m_fpsTimer.start(s_fpsCheckTimeout);
    m_scrollTimer.start(s_scrolTimeout);        
}

void AutoScrollTest::scroll()
{
    PannableViewport* viewport = m_browsingView->pannableViewport();
    QPointF panPos(viewport->panPos().x(), viewport->panPos().y() - s_scrollPixels[m_scrollIndex]);
    viewport->setPanPos(panPos);

    if (m_lastPos.y() == viewport->panPos().y()) {
        m_fpsValues.append(-1);
        if (++m_scrollIndex >= sizeof(s_scrollPixels) / sizeof(int)) {
            m_scrollTimer.stop();
            m_fpsTimer.stop();
            //
            m_fpsResultView = new FPSResultView(m_fpsValues);
            m_fpsResultView->setGeometry(m_browsingView->pannableViewport()->rect());
            m_fpsResultView->appear(m_browsingView->applicationWindow());
            connect(m_fpsResultView, SIGNAL(finished()), this, SLOT(fpsViewClicked()));
        }
    }
    m_lastPos = viewport->panPos();
}

void AutoScrollTest::fpsTick()
{
    int prevfps = m_fpsTicks;
    m_fpsTicks = ((WebView*)((WebViewport*)m_browsingView->pannableViewport())->viewportWidget()->webView())->fpsTicks();

    if (!prevfps)
        return;

    double dt = m_fpsTimestamp.restart();
    double dticks = m_fpsTicks - prevfps;
    if (dt)
        m_fpsValues.append((dticks *  1000.) / dt);
}

void AutoScrollTest::loadFinished(bool success)
{
    if (success)
        starScrollTest();
}

void AutoScrollTest::fpsViewClicked()
{
    emit finished();
}

#include "AutoScrollTest.moc"
