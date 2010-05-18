#ifndef TileSelectionViewBase_h_
#define TileSelectionViewBase_h_

#include <QGraphicsWidget>

class QGraphicsPixmapItem;
class ApplicationWindow;
class TileItem;

class TileSelectionViewBase : public QGraphicsWidget {
    Q_OBJECT
public:
    enum ViewType {
        Home,
        UrlPopup
    };

    virtual ~TileSelectionViewBase();
    void setGeometry( const QRectF & );
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    ViewType viewtype() const { return m_type; }

Q_SIGNALS:
    void appeared();
    void disappeared(TileSelectionViewBase*);

public Q_SLOTS:
    void appear(ApplicationWindow*);
    void disappear();
    virtual void tileItemActivated(TileItem*);
    virtual void tileItemClosed(TileItem*) {}
    virtual void tileItemEditingMode(TileItem*) {}

protected:
    TileSelectionViewBase(ViewType type, QPixmap* bckg, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);

    virtual void createViewItems() = 0;
    virtual void destroyViewItems() = 0;
    virtual void connectItem(TileItem&);
    QGraphicsPixmapItem* m_bckg;

private Q_SLOTS:
    void deleteView();

private:
    ViewType m_type;
};
#endif
