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

#ifndef AutoSelectLineEdit_h
#define AutoSelectLineEdit_h

#include "yberconfig.h"

#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QLineEdit>
#include <QTimer>

class AutoSelectLineEditPrivate;
class KeypadWidget;
class PopupView;

class AutoSelectLineEdit : public QGraphicsWidget
{
    Q_OBJECT
public:
    AutoSelectLineEdit(QGraphicsItem* parent);
    ~AutoSelectLineEdit();

    QString text();

    int selectionStart() const;
    void setCursorPosition(int pos);
    void setSelection(int start, int length);

    void togglePopup();
    bool popupOn() const { return m_virtualKeypad || m_urlfilterPopup; }
    bool virtualKeypadOn() const { return m_virtualKeypad; }
    bool urlFilterPopupOn() const { return m_urlfilterPopup; }

public Q_SLOTS:
    void setText(const QString& text);

Q_SIGNALS:
    void focusChanged(bool);
    void textEdited(const QString&);
    void textEditingFinished(const QString&);

protected Q_SLOTS:
    void popupItemSelected(const QUrl&);
    void popupDismissed();

    void keyPadCharEntered(char key);
    void keyPadBackspace();
    void keyPadEnter();
    void keyPadDotcom();
    void keypadDismissed();

    void returnPressed();

protected:
    bool realFocusEvent();
    void newEditedText(const QString& newText);

private:
    void createVirtualKeypad();
    void createUrlFilterPopup();
    QRectF clientRect();
    void cleanupAndSendFinished(const QString& text);

private:
    friend class AutoSelectLineEditPrivate;
    AutoSelectLineEditPrivate* d;
    KeypadWidget* m_virtualKeypad;
    PopupView* m_urlfilterPopup;
};

#endif
