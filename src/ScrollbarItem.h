#ifndef ScrollbarItem_h_
#define ScrollbarItem_h_

#include <QObject>
#include <QRectF>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QPropertyAnimation>

class QGraphicsWidget;


class ScrollbarItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ScrollbarItem(Qt::Orientation orientation, QGraphicsItem* parent = 0);
    ~ScrollbarItem();

    void updateVisibilityAndFading(bool shouldFadeOut);

    void contentPositionUpdated(qreal pos, qreal contentLength, const QSizeF& viewSize, bool shouldFadeOut);

protected Q_SLOTS:
    void startFadeOut();
    void fadingFinished();

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void startFading(bool in);

private:
    Qt::Orientation m_orientation;
    QPropertyAnimation m_fadeAnim;
    QTimer m_fadeOutTimeout;
};

#endif
