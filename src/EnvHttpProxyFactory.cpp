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

#include "EnvHttpProxyFactory.h"
#include "Helpers.h"

#include <QUrl>

EnvHttpProxyFactory::EnvHttpProxyFactory()
{

}

bool EnvHttpProxyFactory::initFromEnvironment()
{
    bool result = false;

    QUrl proxyUrl = urlFromUserInput(qgetenv("http_proxy"));
    if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
        int proxyPort = (proxyUrl.port() > 0) ? proxyUrl.port() : 8080;
        m_httpProxy << QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyPort);
        result = true;
    } else {
        m_httpProxy << QNetworkProxy::NoProxy;
    }

    proxyUrl = urlFromUserInput(qgetenv("https_proxy"));
    if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
        int proxyPort = (proxyUrl.port() > 0) ? proxyUrl.port() : 8080;
        m_httpsProxy << QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyPort);
        result = true;
    } else {
        m_httpsProxy << QNetworkProxy::NoProxy;
    }
    return result;
}

QList<QNetworkProxy> EnvHttpProxyFactory::queryProxy(const QNetworkProxyQuery & query)
{

    QString protocol = query.protocolTag().toLower();
    if (protocol == QLatin1String("http"))
        return m_httpProxy;
    else if (protocol == QLatin1String("https"))
        return m_httpsProxy;

    QList<QNetworkProxy> result;
    result << QNetworkProxy::NoProxy;
    return result;
}
