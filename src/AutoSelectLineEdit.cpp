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

#include "AutoSelectLineEdit.h"

// timeout between clicking url bar and marking the url selected
static const int s_urlTapSelectAllTimeout = 200;

/*! \class AutoSelectLineEdit input element (\QLineEdit) that selects
  its contents when focused in.

  Needed, because I did not find the feature from stock Qt widgets.
 */
AutoSelectLineEdit::AutoSelectLineEdit(QWidget* parent)
    : QLineEdit(parent)
    , m_selectURLTimer(this)
{
    m_selectURLTimer.setSingleShot(true);
    m_selectURLTimer.setInterval(s_urlTapSelectAllTimeout);
    connect(&m_selectURLTimer, SIGNAL(timeout()), this, SLOT(selectAll()));
}

void AutoSelectLineEdit::focusInEvent(QFocusEvent*e)
{
    QLineEdit::focusInEvent(e);
    m_selectURLTimer.start();
    emit focusChanged(true);
}

void AutoSelectLineEdit::focusOutEvent(QFocusEvent*e)
{
    QLineEdit::focusOutEvent(e);
    m_selectURLTimer.stop();
    emit focusChanged(false);
    deselect();
}
