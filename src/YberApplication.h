#ifndef YberApplication_h
#define YberApplication_h

#include "yberconfig.h"

#include "ApplicationWindow.h"
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

#include "CookieJar.h"

#if USE_DUI
#include <DuiApplication>
typedef DuiApplication YberApplicationBase;
#else
#include <QApplication>
typedef QApplication YberApplicationBase;
#endif

class YberApplication : public YberApplicationBase
{
public:
    explicit YberApplication(int & argc, char ** argv);
    ~YberApplication();

    void start();

    void createMainView(const QUrl& url);

    bool isFullscreen() const { return m_isFullscreen; }
    bool isTileCacheEnabled() const { return m_isTileCacheEnabled; }

    CookieJar* cookieJar() const;

    static YberApplication* instance()
    { return static_cast<YberApplication*>(QCoreApplication::instance()); }

private:
    Q_DISABLE_COPY(YberApplication);

    bool m_isFullscreen;
    bool m_isTileCacheEnabled;
    ApplicationWindow *appwin;
    mutable CookieJar* m_cookieJar;
};





#endif
