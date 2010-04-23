#ifndef TileItem_h_
#define TileItem_h_

#include <QObject>
#include <QRectF>
#include <QGraphicsRectItem>
#include <QLinearGradient>

class QGraphicsWidget;
class UrlItem;

class TileItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    TileItem(QGraphicsWidget* parent, UrlItem* urlItem, bool textOnly);
    ~TileItem();
    
    void setGeometry(const QRectF& rect);

public Q_SLOTS:
    void thumbnailChanged() { update(); }

Q_SIGNALS:
    void itemActivated(UrlItem*);

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void addDropShadow(QPainter& painter, const QRectF rect);

    UrlItem* m_urlItem;
    QRect m_thumbnailRect;
    QString m_title;
    bool m_textOnly;
    bool m_selected;
    QLinearGradient m_bckgGradient;
};

#endif
