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

#include "KeypadWidget.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QPainter>

#include <QDebug>

const int s_keypadMargin = 5;
const QSize s_keypadItemSize = QSize(60, 80);
const QSize s_keypadSpecialItemSize = QSize(80, 80);
const int s_keypadItemMargin = 2;

const int s_keypadCols = 10;
const int s_keypadRows = 3;
const char s_keypad[][10][2] = {{{'q', '1'}, {'w', '2'}, {'e','3'}, {'r','4'}, {'t','5'}, {'y','6'}, {'u','7'}, {'i','8'}, {'o','9'}, {'p','0'}}, 
                                {{'a', '~'}, {'s', '!'}, {'d','@'}, {'f','#'}, {'g','$'}, {'h','%'}, {'j','^'}, {'k','&'}, {'l','*'}, {':',';'}}, 
                                {{'z', '('}, {'x', ')'}, {'c','-'}, {'v','_'}, {'b','='}, {'n','+'}, {'m','?'}, {',','|'}, {'.','\"'}, {'/','\\'}}};

const int s_capslock = -1;
const int s_numpad = -2;
const int s_space = -4;
const int s_backspace = -5;
const int s_enter = -6;
const int s_keypadSpecialCols = 5;
const int s_specialKeypadLine[] = {s_capslock, s_numpad, s_space, s_backspace, s_enter};

class KeypadItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    KeypadItem(uint index, const QRectF& rect, QGraphicsItem* parent = 0);
    ~KeypadItem();

    void toggleCaps();
    void flip();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);

Q_SIGNALS:
    void itemPressed(char character);

private:
    char getchar();

private:
    int m_index;
    int m_pressed;
    bool m_capsOn;
    bool m_flipped;
    QImage* m_image;
    QLinearGradient m_bckgGradient;
};

KeypadItem::KeypadItem(uint index, const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)
    , m_index(index)
    , m_pressed(false)
    , m_capsOn(false)
    , m_flipped(false)
    , m_image(0)
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(120, 120, 120, 240)) << QGradientStop(0.10, QColor(253, 253, 253, 245)) << QGradientStop(0.90, QColor(213, 213, 213, 245));
    for (int j = 0; j < stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);

    QString imagePath;
    if (m_index == s_capslock)
        imagePath = ":/data/icon/48x48/capslock_48.png";
    else if (m_index == s_backspace)
        imagePath = ":/data/icon/48x48/backspace_48.png";
    else if (m_index == s_enter)
        imagePath = ":/data/icon/48x48/enter_48.png";

    if (!imagePath.isEmpty())
        m_image = new QImage(imagePath);

}

KeypadItem::~KeypadItem()
{
    delete m_image;
}

void KeypadItem::toggleCaps() 
{ 
    m_capsOn = !m_capsOn; 
    update();
}

void KeypadItem::flip() 
{ 
    m_flipped = !m_flipped; 
    update(); 
}

void KeypadItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    QRectF r(rect());
    r.adjust(s_keypadItemMargin, s_keypadItemMargin, -s_keypadItemMargin, -s_keypadItemMargin);

    m_bckgGradient.setStart(r.topLeft());
    m_bckgGradient.setFinalStop(r.bottomLeft());

    painter->setPen(QColor(80, 80, 80, 220));
    painter->setBrush(m_bckgGradient);
    painter->drawRoundedRect(r, 5, 5);

    if (m_pressed) {
        painter->setBrush(QColor(90, 90, 90, 100));
        painter->drawRoundedRect(r, 5, 5);
    }
    QFont f(QFont("Nokia Sans", 10));
    f.setBold(true);
    painter->setFont(f);
    painter->setPen(Qt::black);
    QString title;
    if (m_index >= 0)
        title = getchar();
    else if (m_index == s_numpad)
        title = m_flipped ? "abc" : "123";

    if (!title.isEmpty())
        painter->drawText(r, Qt::AlignHCenter|Qt::AlignVCenter, title);
    else if (m_image) {
        QPointF p(rect().left() + rect().width()/2 - m_image->size().width()/2, rect().top() + rect().height()/2 - m_image->size().height()/2);
        painter->drawImage(p, *m_image);
    }
}

void KeypadItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    m_pressed = true;
    update();
}

void KeypadItem::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    m_pressed = false;
    update();
    emit itemPressed(getchar());
}

char KeypadItem::getchar()
{
    if (m_index < 0)
        return m_index;
    char character = s_keypad[m_index/s_keypadCols][m_index%s_keypadCols][m_flipped ? 1 : 0];
    if (m_capsOn && character >= 'a' && character <= 'z')
        character-=32;
    return character;
}

KeypadWidget::KeypadWidget(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)    
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

KeypadWidget::~KeypadWidget()
{
}

void KeypadWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(80, 80, 80, 220));
    painter->drawRect(rect());

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(20, 20, 20, 120));
    painter->drawRoundedRect(m_keypadRect, 5, 5);
}

void KeypadWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
}

void KeypadWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    emit dismissed();
}

void KeypadWidget::appear(int stickyY)
{
    m_keypadRect = QRectF(0, 0, 0, 0);
    int keypadWidth = 2* s_keypadMargin + (s_keypadCols * s_keypadItemSize.width());
    m_keypadRect.setLeft((parentWidget()->rect().width() - keypadWidth) / 2);
    m_keypadRect.setWidth(keypadWidth);
    int keypadHeight = 2* s_keypadMargin + ((s_keypadRows + 1) * s_keypadItemSize.height());
    m_keypadRect.setHeight(keypadHeight);
    m_keypadRect.moveBottom(stickyY);
    layoutKeypad();
}

void KeypadWidget::keypadItemPressed(char character)
{
    if (character == s_numpad) {
        for (int i = 0; i < m_buttons.size(); ++i)
            m_buttons.at(i)->flip();
    } else if (character == s_capslock) {
        for (int i = 0; i < m_buttons.size(); ++i)
            m_buttons.at(i)->toggleCaps();
    } else if (character == s_backspace) {
        emit backspace();
    } else if (character == s_enter) {
        emit enter();
    } else if (character == s_space) {
        emit charEntered(32);
    } else {
        emit charEntered(character);
    }
}

void KeypadWidget::layoutKeypad()
{
    int x = m_keypadRect.left() + s_keypadMargin;
    int y = m_keypadRect.top() + s_keypadMargin;

    for (int i = 0; i < s_keypadRows; ++i) {
        for (int j = 0; j < s_keypadCols; ++j) {
            KeypadItem* item = new KeypadItem(j + s_keypadCols*i, QRectF(QPointF(x, y), s_keypadItemSize), this);
            connect(item, SIGNAL(itemPressed(char)), SLOT(keypadItemPressed(char)));
            m_buttons.append(item);
            x+= s_keypadItemSize.width();
        }
        x = m_keypadRect.left() + s_keypadMargin;
        y+= s_keypadItemSize.height();
    }

    for (int i = 0; i < s_keypadSpecialCols; ++i) {
        // special handling for space, it takes over the leftover place, in the middle
        QSizeF itemSize(s_keypadSpecialItemSize);
        if (s_specialKeypadLine[i] == s_space) {
            int lo = m_keypadRect.right() - x - s_keypadMargin - ((s_keypadSpecialCols - i - 1)*s_keypadSpecialItemSize.width());
            itemSize.setWidth(lo);
        }
        KeypadItem* item = new KeypadItem(s_specialKeypadLine[i], QRectF(QPointF(x, y), itemSize), this);
        connect(item, SIGNAL(itemPressed(char)), SLOT(keypadItemPressed(char)));
        m_buttons.append(item);
        x+= itemSize.width();
    }
}

#include "KeypadWidget.moc"
