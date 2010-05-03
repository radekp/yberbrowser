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
    TileItem(QGraphicsWidget* parent, UrlItem& urlItem, TileLayout layout, bool editable = true);
    ~TileItem();
    
    void setGeometry(const QRectF& rect);
    QRectF geometry() { return rect(); }
    UrlItem* urlItem() const { return m_urlItem; }

    void setEditMode(bool on);
    void setContext(void* context) { m_context = context; }
    void* context() const { return m_context; }

public Q_SLOTS:
    void thumbnailChanged();
    void invalidateClick();

Q_SIGNALS:
    void itemActivated(TileItem*);
    void itemClosed(TileItem*);
    void itemEditingMode(TileItem*);

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
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
    QImage* m_closeIcon;
    QRectF m_closeIconRect;
    bool m_dclick;
    bool m_editable;
    void* m_context; 
};

typedef QList<TileItem*> TileList;

#endif
