#ifndef HistortItem_h_
#define HistortItem_h_

#include <QObject>
#include <QRectF>

class MainWindow;
class QGraphicsWidget;
class UrlItem;

class ThumbnailGraphicsItem;

class HistoryItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    HistoryItem(QGraphicsWidget* parent, UrlItem* urlItem);
    ~HistoryItem();
    
    void setPos(const QPointF& pos);    
    QPointF pos() const;

    QRectF geometry() const;
    void setGeometry(const QRectF& rect);
    UrlItem* urlItem() const { return m_urlItem; }

Q_SIGNALS:
    void itemActivated(UrlItem*);

public Q_SLOTS:
    void thumbnailClicked();

private:
    ThumbnailGraphicsItem* m_thumbnailRect;
    UrlItem* m_urlItem;
};

#endif
