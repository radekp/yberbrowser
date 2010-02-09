#ifndef ScrollbarItem_h_
#define ScrollbarItem_h_

#include <QObject>
#include <QRectF>
#include <QGraphicsRectItem>
#include <QLinearGradient>
#include <QTimer>

class QGraphicsWidget;
class QPropertyAnimation;

class ScrollbarItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ScrollbarItem(QGraphicsItem* parent, bool horizontal);
    ~ScrollbarItem();

    void show();

protected Q_SLOTS:
    void fadeScrollbar();
    void fadingFinished();

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void startFading(bool in);

    QTimer m_visibilityTimer;
    QLinearGradient m_bckgGradient;
    bool m_horizontal;
    QPropertyAnimation* m_fader;
    bool m_fadingIn;
};

#endif
