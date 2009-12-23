#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QNetworkRequest>
#include <QTextStream>
#include <QVector>
#include <QtGui>
#include <QtNetwork/QNetworkProxy>
#include <cstdio>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>
#include <QGLWidget>

#include "MainView.h"

MainView::MainView(QWidget* parent, Settings settings)
    : QGraphicsView(parent)
    , m_mainWidget(0)
    , m_settings(settings)
{
    if (m_settings.m_useGL)
        setViewport(new QGLWidget);

    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setInteractive(false);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);

    if (!m_settings.m_disableTiling) {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    } else {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void MainView::setMainWidget(QGraphicsWidget* widget)
{
    QRectF rect(QRect(QPoint(0, 0), size()));
    widget->setGeometry(rect);
    m_mainWidget = widget;
}

QGraphicsWidget* MainView::mainWidget()
{
    return m_mainWidget;
}

void MainView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    if (!m_mainWidget)
        return;

    if (m_settings.m_disableTiling) {
        QRectF rect(QPoint(0, 0), event->size());
        m_mainWidget->setGeometry(rect);
    }
}
