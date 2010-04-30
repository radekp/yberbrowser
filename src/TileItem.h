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
    Q_PROPERTY(QRectF geometry READ geometry WRITE setGeometry)
public:
    enum TileLayout {
        Horizontal,
        Vertical
    };
    TileItem(QGraphicsWidget* parent, UrlItem& urlItem, TileLayout layout);
    ~TileItem();
    
    void setGeometry(const QRectF& rect);
    QRectF geometry() { return rect(); }
    UrlItem* urlItem() const { return m_urlItem; }

    void setEditMode(bool on);

public Q_SLOTS:
    void thumbnailChanged();

Q_SIGNALS:
    void itemActivated(UrlItem*);
    void itemEdited(UrlItem*);

private:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void setEditIconRect();
    void addDropShadow(QPainter& painter, const QRectF rect);

private:
    UrlItem* m_urlItem;
    QRectF m_thumbnailRect;
    QRectF m_textRect;
    QString m_title;
    TileLayout m_layout;
    bool m_selected;
    QLinearGradient m_bckgGradient;
    QImage* m_defaultIcon;
    QImage* m_editIcon;
    QRectF m_editIconRect;
};

#endif
