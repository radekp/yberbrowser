#ifndef BackingStoreVisualizerWidget_h
#define BackingStoreVisualizerWidget_h

#include <QGraphicsWidget>
#include <QMap>
#include "yberconfig.h"

class QGraphicsWebView;
class TileItem;

class BackingStoreVisualizerWidget : public QGraphicsWidget
{
    Q_OBJECT

public:
    BackingStoreVisualizerWidget(QGraphicsWebView*, QGraphicsItem* parent=0);
    ~BackingStoreVisualizerWidget();

protected Q_SLOTS:
    void tileCreated(unsigned hPos, unsigned vPos);
    void tileRemoved(unsigned hPos, unsigned vPos);
    void tilePainted(unsigned hPos, unsigned vPos);
    void tileCacheViewportScaleChanged();
    void resetCacheTiles();

private:
    Q_DISABLE_COPY(BackingStoreVisualizerWidget);

    void connectSignals();
    void disconnectSignals();
    
    QGraphicsWebView* m_webView;
    QMap<int, TileItem*> m_tileMap;

};

#endif
