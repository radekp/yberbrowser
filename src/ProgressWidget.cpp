#include "ProgressWidget.h"
#include <QDebug>
#include <qgraphicswebview.h>
#include <QPropertyAnimation>

#include "WebViewportItem.h"

static QString s_initialProgressText("Loading...");
static qreal s_progressbarOpacity = 0.8;

class ProgressItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ProgressItem(const QRectF& rect, ProgressWidget& parent) { m_rect = rect; m_parent = &parent; }

    void setOpacity(qreal opacity) { m_opacity = opacity; m_parent->update();}
    qreal opacity() const { return m_opacity; }
    void setRect(const QRectF& rect) { m_rect = rect; }
    QRectF rect() const { return m_rect; }

private:
    ProgressWidget* m_parent;
    qreal m_opacity;
    QRectF m_rect;
};

ProgressWidget::ProgressWidget(WebViewportItem* parent)
    : QGraphicsRectItem(parent)
    , m_lastPercentage(0)
{
    connect(parent->webView()->page(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(parent->webView()->page(), SIGNAL(loadProgress(int)), this, SLOT(progressChanged(int)));
    connect(parent->webView()->page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    sizeChanged();
}

ProgressWidget::~ProgressWidget()
{
    for (int i = m_progressItemList.size() - 1; i >= 0 ; i--)
        delete m_progressItemList.takeAt(i);
}

void ProgressWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    paintBackground(painter);
    // paint percentage items
    int i = 0;
    // should merge them, really
    if (m_progressItemList.size() > 1) {
        QRectF rect = m_progressItemList.at(0)->rect();
        for (;i < m_progressItemList.size(); ++i) {
            if (m_progressItemList.at(i)->opacity() != s_progressbarOpacity)
                break;
            rect.setRight(m_progressItemList.at(i)->rect().right());
        }
        if (i > 0)
            paintItems(painter, rect, s_progressbarOpacity);
    }

    for (int j = i; j < m_progressItemList.size(); ++j) 
        paintItems(painter, m_progressItemList.at(j)->rect(), m_progressItemList.at(j)->opacity());

    // paint percentage
    painter->setOpacity(1.0);
    painter->setPen(QColor(40, 40, 40));
    painter->drawText(progressBoxRect(), Qt::AlignCenter, m_label);
}

void ProgressWidget::paintBackground(QPainter* painter)
{
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(120, 120, 120));
    stops << QGradientStop(0.30, QColor(209, 204, 234));
    stops << QGradientStop(0.50, QColor(219, 214, 244));
    stops << QGradientStop(0.70, QColor(209, 204, 234));
    stops << QGradientStop(1.00, QColor(120, 120, 120));

    QLinearGradient g(progressBoxRect().topLeft(), progressBoxRect().bottomLeft());

    for (int j=0; j<stops.size(); ++j)
        g.setColorAt(stops.at(j).first, stops.at(j).second);

    painter->setBrush(g);
    painter->setPen(Qt::black);
    painter->setOpacity(0.8);
    painter->drawRoundedRect(progressBoxRect(), 3, 1);
}

void ProgressWidget::paintItems(QPainter* painter, const QRectF& rect, qreal opacity)
{
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(226, 255, 51));
    stops << QGradientStop(0.50, QColor(39, 115 , 3));
    stops << QGradientStop(0.60, QColor(39, 115 , 3));
    stops << QGradientStop(1.00, QColor(226, 255, 51));

    QLinearGradient gb(rect.topLeft(), rect.bottomLeft());

    for (int j=0; j<stops.size(); ++j)
        gb.setColorAt(stops.at(j).first, stops.at(j).second);

    painter->setBrush(gb);
    painter->setPen(Qt::NoPen);
    painter->setOpacity(opacity);
    painter->drawRect(rect);
}
   
void ProgressWidget::loadStarted()
{
    m_label = s_initialProgressText;
    m_lastPercentage = 0;
    show();
}

void ProgressWidget::progressChanged(int percentage)
{
    // todo: find out this magic 10% thing
    if (percentage == m_lastPercentage || percentage <= 10)
        return;
    
    m_lastPercentage = percentage;
    
    m_label = QString::number(percentage) + "%";

    // create the new progress item
    QRectF rect(progressBoxRect());
    // shrink it
    rect.setY(rect.y() + 1);
    rect.setBottom(rect.bottom() - 1);
    int startX = rect.x() + 1;

    if (m_progressItemList.size())
        startX = m_progressItemList.at(m_progressItemList.size() - 1)->rect().right();

    rect.setRight(rect.x() + (rect.width() * percentage) / 100);
    rect.setLeft(startX);

    ProgressItem* item = new ProgressItem(rect, *this);
    m_progressItemList.append(item);

    // assign opacity animation
    QPropertyAnimation* animation = new QPropertyAnimation(item, "opacity");
    animation->setDuration(800);

    animation->setStartValue(0.0);
    animation->setEndValue(s_progressbarOpacity);

    animation->setEasingCurve(QEasingCurve::InCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ProgressWidget::loadFinished(bool /*success*/)
{
    hide();
    m_lastPercentage = 0;
    for (int i = m_progressItemList.size() - 1; i >= 0 ; --i)
        delete m_progressItemList.takeAt(i);
}

void ProgressWidget::sizeChanged()
{
    const QFont f = QFont();
    QFontMetrics fm(f);
    int height = fm.height();
    // set the progressbox size to 1/4 of the view
    int width = rect().width();
    // todo: progress width stays the same while loading. make it dynamic 
    if (m_lastPercentage == 0)
        width = qMax(fm.size(Qt::TextSingleLine, s_initialProgressText).width() + 30, int(parentWidget()->rect().width() / 4));
    m_progressBoxRect = QRectF(0, parentWidget()->rect().bottomLeft().y() - (height + 3), width, height + 3);
    // reset progress items
    for (int i = 0; i < m_progressItemList.size(); ++i) {
        QRectF r = m_progressItemList.at(i)->rect();
        r.setY(m_progressBoxRect.y() + 1);
        r.setBottom(m_progressBoxRect.bottom() - 1);
        m_progressItemList.at(i)->setRect(r);
    }

    setRect(m_progressBoxRect);
}

QRectF ProgressWidget::progressBoxRect()
{
    return m_progressBoxRect;
}

#include "ProgressWidget.moc"
