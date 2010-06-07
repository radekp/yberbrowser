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

#ifndef KeypadWidget_h_
#define KeypadWidget_h_

#include <QGraphicsWidget>
#include <QLinearGradient>
#include <QImage>
#include <QTimer>

class PopupView;
class KeypadItem;

class KeypadWidget : public QGraphicsWidget {
    Q_OBJECT
public:
    KeypadWidget(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    ~KeypadWidget();

    void appear(int stickyY);

Q_SIGNALS:
    void textEntered(const QString& text);
    void charEntered(char key);
    void backspace();
    void enter();
    void dismissed();
    void pageSelected(const QUrl&);

public Q_SLOTS:
    // FIXME: keypad should really know what's been entered to update popup
    void updatePopup(const QString&);

protected:
    void resizeEvent(QGraphicsSceneResizeEvent* event);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

protected Q_SLOTS:
    void keypadItemPressed(KeypadItem& item);
    void keypadItemSelected(KeypadItem& item);
    void cancelBubble();

    void popupItemSelected(const QUrl&);
    void popupDismissed();

private:
    void layoutKeypad();

private:
    PopupView* m_urlfilterPopup;
    QList<KeypadItem*> m_buttons;
    KeypadItem* m_prevButton;
    QTimer m_bubbleTimer;
    QRectF m_keypadRect;
    QImage m_bubbleIcon;
};
#endif

