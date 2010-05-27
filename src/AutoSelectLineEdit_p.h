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

class AutoSelectLineEditPrivate : public QLineEdit
{
    Q_OBJECT
public:
    AutoSelectLineEditPrivate(AutoSelectLineEdit* qq)
        : QLineEdit()
        , q(qq)
        , selectURLTimer(this)
    {
        selectURLTimer.setSingleShot(true);
        selectURLTimer.setInterval(s_urlTapSelectAllTimeout);
        connect(&selectURLTimer, SIGNAL(timeout()), this, SLOT(selectAll()));

        connect(this, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));
        connect(this, SIGNAL(returnPressed()), q, SIGNAL(returnPressed()));
    }

    void adjustText()
    {
        if (!hasFocus() && url.startsWith("http://")) {
            QString text = url;
            setText(text.remove(0, 7));
        } else
            setText(url);
    }

public Q_SLOTS:
    void onTextEdited(const QString& newText)
    {
        url = newText;
        emit q->textEdited(newText);
    }

protected:
    virtual void focusInEvent(QFocusEvent* e)
    {
        QLineEdit::focusInEvent(e);
        adjustText();
        selectURLTimer.start();
        emit q->focusChanged(true);
    }

    virtual void focusOutEvent(QFocusEvent* e)
    {
        QLineEdit::focusOutEvent(e);
        adjustText();
        selectURLTimer.stop();
        emit q->focusChanged(false);
        deselect();
    }

    friend class AutoSelectLineEdit;
    AutoSelectLineEdit* q;
    QTimer selectURLTimer;
    QString url;
};



