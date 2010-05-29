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
#include <QImage>
#include <QPainter>
#include <QDebug>

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
        , m_keyboardIcon(QImage(":/data/icon/32x32/keyboard_32.png"))
        , m_popupIcon(QImage(":/data/icon/32x32/popup_32.png"))
        , m_focusedEditor(false)
        
    {
        selectURLTimer.setSingleShot(true);
        selectURLTimer.setInterval(s_urlTapSelectAllTimeout);
        connect(&selectURLTimer, SIGNAL(timeout()), this, SLOT(selectAll()));

        connect(this, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));
        connect(this, SIGNAL(returnPressed()), q, SLOT(returnPressed()));
        // margin for the icon
        setTextMargins(0, 0, m_keyboardIcon.size().width(), 0);
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
        q->newEditedText(newText);
    }

protected:

    virtual void paintEvent(QPaintEvent* event) 
    {
        QLineEdit::paintEvent(event);
        if (hasFocus() || q->popupOn()) {
            QPainter painter(this);
            
            QPointF p(rect().right() - m_keyboardIcon.size().width(), rect().height()/2 - m_keyboardIcon.size().height()/2);
            painter.drawImage(p, q->virtualKeypadOn() ? m_popupIcon : m_keyboardIcon);
        }
    }

    virtual void mousePressEvent(QMouseEvent* e)
    {
        QRectF r(rect());
        r.setLeft(r.right() - m_keyboardIcon.size().width());
        if (!r.contains(e->pos())) {
            QLineEdit::mousePressEvent(e);
        }
    }

    virtual void mouseReleaseEvent(QMouseEvent* e)
    {
        QRectF r(rect());
        r.setLeft(r.right() - m_keyboardIcon.size().width());
        if (r.contains(e->pos())) {
            q->togglePopup();
            update();
        } else {
            QLineEdit::mouseReleaseEvent(e);
        }
    }

    virtual void focusInEvent(QFocusEvent* e)
    {
        if (q->realFocusEvent() && !m_focusedEditor) {
            m_focusedEditor = true;
            QLineEdit::focusInEvent(e);
            adjustText();
            selectURLTimer.start();
            emit q->focusChanged(true); 
        }
    }

    virtual void focusOutEvent(QFocusEvent* e)
    {
        if (q->realFocusEvent() && m_focusedEditor) {
            m_focusedEditor = false;
            QLineEdit::focusOutEvent(e);
            adjustText();
            selectURLTimer.stop();
            emit q->focusChanged(false);
            deselect();
        }
    }

    friend class AutoSelectLineEdit;
    AutoSelectLineEdit* q;
    QTimer selectURLTimer;
    QString url;
    QImage m_keyboardIcon;
    QImage m_popupIcon;
    // need manual book-keeping because of the hackish focus handling
    bool m_focusedEditor;
};



