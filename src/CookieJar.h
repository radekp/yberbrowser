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

#ifndef CookieJar_h_
#define CookieJar_h_

#include <QNetworkCookieJar>
#include <QBasicTimer>

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    CookieJar(QObject* parent = 0);
    virtual ~CookieJar();

    void save();
    void load();

    virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

protected:
    virtual void timerEvent(QTimerEvent* ev);

private:
    void expireCookies();
    QBasicTimer m_cookieSavingTimer;
    bool m_cookiesChanged;
};

#endif

