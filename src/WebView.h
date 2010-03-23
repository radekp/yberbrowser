#ifndef WebView_h
#define WebView_h
#include <qgraphicswebview.h>
#include <QDebug>
#include "yberconfig.h"
#include "PannableViewport.h"

class WebView : public QGraphicsWebView {
    Q_OBJECT

public:
    WebView(QGraphicsItem* parent = 0);

    void paint(QPainter* p, const QStyleOptionGraphicsItem* i, QWidget* w= 0) {
        m_fpsTicks++;
        QGraphicsWebView::paint(p, i, w);
    }

    unsigned int fpsTicks() const { return m_fpsTicks; }


private:
    Q_DISABLE_COPY(WebView);

    unsigned int m_fpsTicks;
};

#endif
