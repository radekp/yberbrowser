#ifndef ProgressWidget_h_
#define ProgressWidget_h_

#include <QObject>
#include <QGraphicsRectItem>

class WebViewportItem;
class ProgressItem;

class ProgressWidget : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    ProgressWidget(WebViewportItem* parent);
    ~ProgressWidget();

    void sizeChanged();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

public Q_SLOTS:
    void loadStarted();
    void progressChanged(int percentage);
    void loadFinished(bool success);

private:
    void paintBackground(QPainter* painter);
    void paintItems(QPainter* painter, const QRectF& rect, qreal opacity);
    QRectF progressBoxRect();

    QString m_label;
    QList<ProgressItem*> m_progressItemList;
    int m_lastPercentage;
    QRectF m_progressBoxRect;
};

#endif
