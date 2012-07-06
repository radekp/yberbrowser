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

#ifndef ApplicationWindow_h
#define ApplicationWindow_h

#include "yberconfig.h"

#if USE_MEEGOTOUCH

#include <MApplicationWindow>
typedef class MApplicationWindow ApplicationWindow;

#else

#include <QGraphicsView>
class QWidget;
class QResizeEvent;
class ApplicationWindowHost;
class QMenuBar;


class ApplicationWindow : public QGraphicsView
{
    Q_OBJECT
public:
    ApplicationWindow(QWidget* parent=0);
    void setPage(QGraphicsWidget* page);

    void setMenuBar(QMenuBar*);
    void show();
    bool isFullScreen();

    void setTitle(const QString& title);

    QGraphicsWidget* currentPage() const { return m_page; }

protected:
    void resizeEvent(QResizeEvent* event);

private:
    ApplicationWindowHost* m_owner;
    QGraphicsWidget* m_page;
    bool m_isFullScreen;

public slots:
    void showMaximized();
    void showFullScreen();
    static void toggleFullScreen();
};

#endif

#endif
