/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "ApplicationWindow.h"
#include "ApplicationWindowHost.h"

#include <QGraphicsWidget>
#include <QMenuBar>

#if QTOPIA
#include <QSoftMenuBar>
#endif

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
            m_page->resize(size());
    }
}

void ApplicationWindow::setMenuBar(QMenuBar* bar)
{
#ifdef QTOPIA
    QMenu *menu = QSoftMenuBar::menuFor(this);
#else
    QMenu *menu = m_owner->menuBar()->addMenu("&File");
#endif
    menu->addAction(tr("Maximize"), this, SLOT(showMaximized()));
    menu->addAction(tr("Fullscreen"), this, SLOT(showFullScreen()));
    m_owner->showMaximized();
}

void ApplicationWindow::show()
{
    m_owner->showNormal();
    m_isFullScreen = false;
}

void ApplicationWindow::showMaximized()
{
    m_owner->showMaximized();
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


