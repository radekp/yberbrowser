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

#ifndef YberApplication_h
#define YberApplication_h

#include "yberconfig.h"

#include "ApplicationWindow.h"
#include "CookieJar.h"

class YberApplication
{
public:
<<<<<<< HEAD
    explicit YberApplication(int& argc, char** argv);
=======
    explicit YberApplication();
>>>>>>> Separate YberApplication and Q/MApplication
    ~YberApplication();

    void start();

    void createMainView(const QUrl& url);

    CookieJar* cookieJar() const;

<<<<<<< HEAD
    static YberApplication* instance()
    {
        return static_cast<YberApplication*>(QCoreApplication::instance());
    }
=======
    static YberApplication* instance();
>>>>>>> Separate YberApplication and Q/MApplication

private:
    Q_DISABLE_COPY(YberApplication)

    ApplicationWindow *m_appwin;
    mutable CookieJar* m_cookieJar;
};

#endif
