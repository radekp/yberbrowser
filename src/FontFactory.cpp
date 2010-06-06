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

#include "FontFactory.h"

#include <QFontDatabase>
#include <QStringList>
#include <QRegExp>
#include <QDebug>

const QString s_defaultFontFamily = QString("Nokia Sans");

// FIXME this needs some proper API. barebone, all we need, impl atm.
FontFactory* FontFactory::instance()
{
    static FontFactory* s_instance = 0;
    if (!s_instance)
        s_instance = new FontFactory();
    return s_instance;
}

FontFactory::FontFactory()
{
    QString fontFamily = s_defaultFontFamily;
    // find something Sansish, if not, pick the first one
    QStringList l = QFontDatabase().families(QFontDatabase::Latin);
    if (!l.contains(s_defaultFontFamily, Qt::CaseInsensitive)) {
        QRegExp rx("*Sans*");
        rx.setPatternSyntax(QRegExp::Wildcard);
        int index = l.indexOf(rx);  
        if (index == -1)
            index = 0;
        fontFamily = l.at(index);
    }

    m_smallFont.setFamily(fontFamily);
    m_mediumFont.setFamily(fontFamily);
    m_bigFont.setFamily(fontFamily);

    int small = 12;
    int medium = 18;
    int big = 30;
#ifdef Q_OS_SYMBIAN
    small = 6;
    medium = 8;
    big = 12;
#endif
    m_smallFont.setPointSize(small);
    m_mediumFont.setPointSize(medium);
    m_bigFont.setPointSize(big);
}
