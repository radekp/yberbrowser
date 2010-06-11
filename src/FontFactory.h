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

#ifndef FontFactory_h_
#define FontFactory_h_

#include <QFont>

class FontFactory {
public:
    static FontFactory* instance();  

    const QFont& small() { return m_smallFont; }
    const QFont& medium() { return m_mediumFont; }
    const QFont& big() { return m_bigFont; }
    
private:
    FontFactory();

    QFont m_smallFont;
    QFont m_mediumFont;
    QFont m_bigFont;
};

#endif
