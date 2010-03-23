#include "WebView.h"

WebView::WebView(QGraphicsItem* parent)
    : QGraphicsWebView(parent)
    , m_fpsTicks(0)
{
}

