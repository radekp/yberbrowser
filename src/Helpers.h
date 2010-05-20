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

#ifndef Helpers_h_
#define Helpers_h_

#include <QUrl>
#include "UrlItem.h"

class QString;
class QGraphicsWidget;
class QGraphicsWebView;

void notification(const QString& text, QGraphicsWidget* parent);
QUrl urlFromUserInput(const QString& string);
void internalizeUrlList(UrlList& list, const QString& fileName, uint version);
void externalizeUrlList(UrlList& list, const QString& fileName, uint version);

#endif
