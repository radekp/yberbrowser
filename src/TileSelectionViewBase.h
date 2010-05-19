#ifndef TileSelectionViewBase_h_
#define TileSelectionViewBase_h_

#include <QGraphicsWidget>

class QGraphicsPixmapItem;
class ApplicationWindow;
class TileItem;
class QGraphicsSceneMouseEvent;

class TileSelectionViewBase : public QGraphicsWidget {
    Q_OBJECT
public:
    enum ViewType {
        Home,
        UrlPopup
    };

    virtual ~TileSelectionViewBase();

    void appear(ApplicationWindow*);
    void disappear();

    void setGeometry( const QRectF & );
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    ViewType viewtype() const { return m_type; }

    void updateBackground(QPixmap* bckg);
    void updateContent();

    // FIXME temp hack until event handling is fixed
    virtual bool filterMouseEvent(QGraphicsSceneMouseEvent*) { return false; }

Q_SIGNALS:
    void viewDismissed();

public Q_SLOTS:
    virtual void tileItemActivated(TileItem*) {}
    virtual void tileItemClosed(TileItem*) {}
    virtual void tileItemEditingMode(TileItem*) {}

protected:
    TileSelectionViewBase(ViewType type, QPixmap* bckg, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);

    virtual void createViewItems() = 0;
    virtual void destroyViewItems() = 0;
    virtual void connectItem(TileItem&);

protected Q_SLOTS:
    void closeView();
    void closeViewSoon();

private:
    QGraphicsPixmapItem* m_bckg;
    ViewType m_type;
};
#endif
