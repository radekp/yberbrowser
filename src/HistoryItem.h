#ifndef HistortItem_h_
#define HistortItem_h_

#include <QObject>
#include <QRectF>
#include <QGraphicsRectItem>

class QGraphicsWidget;
class UrlItem;

class HistoryItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    HistoryItem(QGraphicsWidget* parent, UrlItem* urlItem);
    ~HistoryItem();
    
    void setGeometry(const QRectF& rect);

public Q_SLOTS:
    void thumbnailChanged() { update(); }

Q_SIGNALS:
    void itemActivated(UrlItem*);

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);

    UrlItem* m_urlItem;
    QRect m_thumbnailRect;
    QPoint m_titlePos;
    QString m_title;
};

#endif
