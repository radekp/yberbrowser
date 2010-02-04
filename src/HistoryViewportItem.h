#ifndef HistoryViewportItem_h_
#define HistoryViewportItem_h_

#include <QGraphicsWidget>
#include <QList>

class HistoryItem;
class PageView;
class TileBackground;
class QParallelAnimationGroup;

class HistoryViewportItem : public QGraphicsWidget {
    Q_OBJECT
public:
    HistoryViewportItem(PageView& view, QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HistoryViewportItem();

    void setGeometry(const QRectF& rect);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void toggleHistory();
    bool isActive() const { return m_active; }

Q_SIGNALS:
    void hideHistory();

public Q_SLOTS:
    void animFinished();
    
private:
    void createHistoryTiles();
    void destroyHistoryTiles();
    void startAnimation(bool in);

private:
    PageView* m_view;
    TileBackground* m_bckg;
    QList<HistoryItem*> m_historyList;
    QParallelAnimationGroup* m_animGroup;
    bool m_active;
    bool m_ongoing;
};

#endif
