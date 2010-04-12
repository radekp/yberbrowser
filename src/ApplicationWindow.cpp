#include <QGraphicsWidget>
#include "ApplicationWindow.h"
#include "ApplicationWindowHost.h"
#include <QMenuBar>

/*!
  \class ApplicationWindow is the top-level application container

  \ApplicationWindow is responsible for showing "application pages",
  single ui views.

  \ApplicationWindow is created by \YberApplication and owned by
  \ApplicationWindowHost
*/
ApplicationWindow::ApplicationWindow(QWidget* parent)
    : QGraphicsView(parent)
    , m_owner(new ApplicationWindowHost())
    , m_page(0)
    , m_isFullScreen(false)
{
    m_owner->setApplicationWindow(this);
}

void ApplicationWindow::setPage(QGraphicsWidget* page)
{
    scene()->addItem(page);
    m_page = page;
    m_page->setGeometry(QRectF(m_page->pos(), size()));
}

void ApplicationWindow::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    if (scene()) {
        QRectF rect(QPointF(0, 0), size());
        scene()->setSceneRect(rect);
        if (m_page)
            m_page->setGeometry(QRectF(m_page->pos(), size()));
    }
}

void ApplicationWindow::setMenuBar(QMenuBar* bar)
{
    m_owner->setMenuBar(bar);
}

void ApplicationWindow::show()
{
    m_owner->showNormal();
    m_isFullScreen = false;
}

void ApplicationWindow::showFullScreen()
{
    m_owner->showFullScreen();
    m_isFullScreen = true;
}

bool ApplicationWindow::isFullScreen()
{
    return m_isFullScreen;
}


void ApplicationWindow::setTitle(const QString& title)
{
    m_owner->setWindowTitle(title);
}


