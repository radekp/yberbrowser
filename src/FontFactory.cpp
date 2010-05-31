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

// FIXME this needs some proper API. barebone, all we need, impl atm.
FontFactory* FontFactory::instance()
{
    FontFactory* s_instance = 0;
    if (!s_instance)
        s_instance = new FontFactory();
    return s_instance;
}

FontFactory::FontFactory()
{
    m_smallFont.setFamily("Nokia Sans");
    m_mediumFont.setFamily("Nokia Sans");
    m_bigFont.setFamily("Nokia Sans");

    int small = 12;
    int medium = 18;
    int big = 30;
#if defined(Q_WS_SYMBIAN)
    small = 8;
    medium = 12;
    big = 18;
#endif
    m_smallFont.setPointSize(small);
    m_mediumFont.setPointSize(medium);
    m_bigFont.setPointSize(big);
}
