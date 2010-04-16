#ifndef HistoryView_h_
#define HistoryView_h_

#include <QGraphicsWidget>
#include <QList>

class HistoryItem;
class QGraphicsRectItem;
class QPropertyAnimation;
class UrlItem;
class ApplicationWindow;

class HistoryView : public QGraphicsWidget {
    Q_OBJECT
public:
    HistoryView(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~HistoryView();

    void setGeometry(const QRectF& rect);

    bool isActive() const { return m_active; }

    void appear(ApplicationWindow *window);
    void disappear();

Q_SIGNALS:
    void disappeared();
    void urlSelected(const QUrl&);

public Q_SLOTS:
    void animFinished();
    void historyItemActivated(UrlItem* item);
    
private:
    void createHistoryTiles();
    void destroyHistoryTiles();
    void startAnimation(bool in);

private:
    QGraphicsRectItem* m_bckg;
    QGraphicsWidget* m_tileContainer;
    QList<HistoryItem*> m_historyList;
    QPropertyAnimation* m_slideAnim;
    bool m_active;
};

#endif
