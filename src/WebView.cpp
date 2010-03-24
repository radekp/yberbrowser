#include "WebView.h"
#if defined(ENABLE_PAINT_DEBUG)
#include <QTime>
#include <QStyleOptionGraphicsItem>
#endif
WebView::WebView(QGraphicsItem* parent)
    : QGraphicsWebView(parent)
    , m_fpsTicks(0)
{
}

void WebView::paint(QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* w)
{
    m_fpsTicks++;
#if defined(ENABLE_PAINT_DEBUG)
    QTime t;
    t.start();
#endif
    QGraphicsWebView::paint(p, option, w);
#if defined(ENABLE_PAINT_DEBUG)
    qDebug() << __FUNCTION__ << "ticks:" << m_fpsTicks << t.elapsed() << option->exposedRect.toAlignedRect();
#endif
}
