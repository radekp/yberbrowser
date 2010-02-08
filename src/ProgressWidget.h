#ifndef ProgressWidget_h_
#define ProgressWidget_h_

#include <QObject>
#include <QGraphicsRectItem>
#include <QLinearGradient>

class WebViewportItem;
class ProgressItem;
class QPropertyAnimation;

class ProgressWidget : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    ProgressWidget(WebViewportItem* parent);
    ~ProgressWidget();

    void udpateGeometry(const QRectF& rect);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

    void setPos(const QPointF& pos);

public Q_SLOTS:
    void loadStarted();
    void progressChanged(int percentage);
    void loadFinished(bool success);
    void slideFinished();

private:
    void paintBackground(QPainter* painter);
    void paintItems(QPainter* painter, const QRectF& rect, qreal opacity);
    QRectF progressBoxRect();
    void slide(bool in);
    void destroyProgressItems();

    QString m_label;
    QList<ProgressItem*> m_progressItemList;
    QPropertyAnimation* m_slider;
    int m_lastPercentage;
    QRectF m_progressBoxRect;
    unsigned m_slideAnimState;
    QLinearGradient m_bckgGradient;
    QLinearGradient m_progressGradient;
};

#endif
