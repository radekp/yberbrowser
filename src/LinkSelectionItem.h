#ifndef LinkSelectionItem_h_
#define LinkSelectionItem_h_

#include <QObject>
#include <QRectF>
#include <QGraphicsRectItem>
#include <QSequentialAnimationGroup>

class QGraphicsItem;

class LinkSelectionItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(QRectF rect READ rect WRITE setRect)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    LinkSelectionItem(QGraphicsItem*);
    void appear(const QPointF&, const QRectF&);

private:
    QSequentialAnimationGroup m_linkSelectiogroup;
};

#endif
