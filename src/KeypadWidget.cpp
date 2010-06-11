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
#include "FontFactory.h"
#include "PopupView.h"
#include "AutoSelectLineEdit.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QUrl>
#include <QDebug>

#ifdef Q_OS_SYMBIAN
const int s_keypadXMargin = 1;
#else
const int s_keypadXMargin = 5;
#endif
const int s_keypadYMargin = 5;
const QSize s_keypadItemMaxSize = QSize(68, 51);
const int s_keypadItemMargin = 1;
// keys width/height proportion
const qreal s_keySizePropotion = 0.75;
// special keys are this much wider
const qreal s_speclialKeySizePropotion = 1.12;
const int s_popupMargin = 4;
const int s_keypadBottomMargin = 2;

const int s_keypadCols = 10;
const int s_keypadRows = 3;
const char s_keypad[][10][2] = {{{'q', '1'}, {'w', '2'}, {'e','3'}, {'r','4'}, {'t','5'}, {'y','6'}, {'u','7'}, {'i','8'}, {'o','9'}, {'p','0'}}, 
                                {{'a', '~'}, {'s', '!'}, {'d','@'}, {'f','#'}, {'g','$'}, {'h','%'}, {'j','^'}, {'k','&'}, {'l','*'}, {':',';'}}, 
                                {{'z', '('}, {'x', ')'}, {'c','-'}, {'v','_'}, {'b','='}, {'n','+'}, {'m','?'}, {',','|'}, {'.','\"'}, {'/','\\'}}};

const int s_capslock = -1;
const int s_numpad = -2;
const int s_space = -4;
const int s_dotcom = -5;
const int s_backspace = -6;
const int s_enter = -7;
const int s_keypadSpecialCols = 6;
const int s_keypadSpecialRows = 1;
const int s_specialKeypadLine[] = {s_capslock, s_numpad, s_space, s_dotcom, s_backspace, s_enter};
const int s_bubbleIconSize = 64;
const int s_bubbleTimeout = 800;

const QString s_dotcomStr = QString(".com");

class KeypadItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    KeypadItem(uint index, const QRectF& rect, QGraphicsItem* parent = 0);
    ~KeypadItem();

    void toggleCaps();
    void flip();
    void setBubble(QImage* bubble);
    int getChar();

Q_SIGNALS:
    void itemSelected(KeypadItem& item);
    void itemPressed(KeypadItem& item);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    QRectF boundingRect () const;

private:
    int m_index;
    int m_pressed;
    bool m_capsOn;
    bool m_flipped;
    QImage* m_image;
    QImage* m_bubbleIcon;
    QLinearGradient m_bckgGradient;
};

KeypadItem::KeypadItem(uint index, const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)
    , m_index(index)
    , m_pressed(false)
    , m_capsOn(false)
    , m_flipped(false)
    , m_image(0)
    , m_bubbleIcon(0)
{
#ifndef Q_OS_SYMBIAN
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif    
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

void KeypadItem::setBubble(QImage* bubble)
{
    // no bubble for special buttons
    if (m_index < 0)
        return;
    m_bubbleIcon = bubble;
    update(boundingRect());
}

void KeypadItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    QRectF r(rect());
    r.adjust(s_keypadItemMargin, s_keypadItemMargin, -s_keypadItemMargin, -s_keypadItemMargin);

    m_bckgGradient.setStart(r.topLeft());
    m_bckgGradient.setFinalStop(r.bottomLeft());

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->setPen(QColor(80, 80, 80, 220));
    painter->setBrush(m_bckgGradient);
    painter->drawRoundedRect(r, 15, 15);

    if (m_pressed) {
        painter->setBrush(QColor(90, 90, 90, 100));
        painter->drawRoundedRect(r, 5, 5);
    }
    painter->setFont(FontFactory::instance()->small());
    painter->setPen(Qt::black);
    QString title;
    if (m_index >= 0)
        title = getChar();
    else if (m_index == s_numpad)
        title = m_flipped ? "abc" : "123";
    else if (m_index == s_dotcom)
        title = s_dotcomStr;

    if (!title.isEmpty())
        painter->drawText(r, Qt::AlignHCenter|Qt::AlignVCenter, title);
    else if (m_image) {
        QRectF ir(rect());
        // image margin
        ir.adjust(5, 5, -5, -5);
        painter->drawImage(ir, *m_image);
    }
    if (m_bubbleIcon) {
        QRectF br(rect());
        br.moveBottom(br.top());
        painter->drawImage(br, *m_bubbleIcon);
        painter->setPen(Qt::white);
        // shape of the bubble icon drives the rectangle of the text
        br.adjust(0, 0, 0, -1*br.height()/4);
        painter->drawText(br, Qt::AlignHCenter|Qt::AlignVCenter, title);
    }
}

void KeypadItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    m_pressed = true;
    update(boundingRect());
    emit itemPressed(*this);
}

void KeypadItem::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
    m_pressed = false;
    update(boundingRect());
    emit itemSelected(*this);
}

QRectF KeypadItem::boundingRect() const
{
    QRectF r(rect());
    // extend paint bounding rect to get the bubble updated
    r.adjust(0, -s_bubbleIconSize, s_bubbleIconSize, 0);
    return r;
}

int KeypadItem::getChar()
{
    if (m_index < 0)
        return m_index;
    char character = s_keypad[m_index/s_keypadCols][m_index%s_keypadCols][m_flipped ? 1 : 0];
    if (m_capsOn && character >= 'a' && character <= 'z')
        character-=32;
    return character;
}

KeypadWidget::KeypadWidget(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)    
    , m_urlfilterPopup(new PopupView(this))
    , m_prevButton(0)
    , m_bubbleIcon(QImage(":/data/icon/64x64/bubble_64.png"))
{
#ifndef Q_OS_SYMBIAN
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif    
    m_bubbleTimer.setSingleShot(true);
    connect(&m_bubbleTimer, SIGNAL(timeout()), SLOT(cancelBubble())); 
    connect(m_urlfilterPopup, SIGNAL(pageSelected(const QUrl&)), SLOT(popupItemSelected(const QUrl&)));
    connect(m_urlfilterPopup, SIGNAL(viewDismissed()), SLOT(popupDismissed()));
    m_urlfilterPopup->appear();
}

KeypadWidget::~KeypadWidget()
{
}

void KeypadWidget::updatePopup(const QString& text)
{
    // signal from the connected url editor
    m_urlfilterPopup->setFilterText(text);
}

void KeypadWidget::resizeEvent(QGraphicsSceneResizeEvent*)
{
}

void KeypadWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(60, 60, 60, 220));
    QRectF r(rect());
    r.setTop(r.top() + m_urlfilterPopup->geometry().height() + s_popupMargin);
    painter->drawRect(r);

    painter->setPen(Qt::white);
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
    m_keypadRect = QRectF(rect());
    int keypadWidth = s_keypadItemMaxSize.width() * s_keypadCols + 2*s_keypadXMargin;
    // too wide??    
    if (keypadWidth > rect().width())
        keypadWidth = rect().width();

    m_keypadRect.setWidth(keypadWidth);
    m_keypadRect.setHeight((m_keypadRect.width() / s_keypadCols) * s_keySizePropotion * (s_keypadRows + s_keypadSpecialRows) + 2*s_keypadYMargin);
    m_keypadRect.moveLeft(rect().left() + (rect().width()/2 - m_keypadRect.width()/2));
    m_keypadRect.moveBottom(stickyY - s_keypadBottomMargin);
    layoutKeypad();
    
    QRectF filterRect(rect());
    filterRect.setTop(filterRect.top() + s_popupMargin);
    filterRect.setHeight(rect().height() - m_keypadRect.height() - 2*s_popupMargin);
    m_urlfilterPopup->setPos(filterRect.topLeft());
    m_urlfilterPopup->resize(filterRect.size());
    m_urlfilterPopup->appear();
}

void KeypadWidget::keypadItemPressed(KeypadItem& item)
{
    if (m_prevButton)
        m_prevButton->setBubble(0);
    item.setBubble(&m_bubbleIcon);
    m_prevButton = &item;
    m_bubbleTimer.start(s_bubbleTimeout);
}

void KeypadWidget::keypadItemSelected(KeypadItem& item)
{
    int character = item.getChar();
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
    } else if (character == s_dotcom) {
        emit textEntered(s_dotcomStr);
    } else {
        emit charEntered(character);
    }
}

void KeypadWidget::cancelBubble()
{
    m_prevButton->setBubble(0);
    m_prevButton = 0;
}

void KeypadWidget::popupItemSelected(const QUrl& url)
{
    emit pageSelected(url);
}

void KeypadWidget::popupDismissed()
{
    emit dismissed();
}

void KeypadWidget::layoutKeypad()
{
    int x = m_keypadRect.left() + s_keypadXMargin;
    int y = m_keypadRect.top() + s_keypadYMargin;
    QSizeF keySize;
    keySize.setWidth((m_keypadRect.width() - 2 * s_keypadXMargin) / s_keypadCols);
    keySize.setHeight(keySize.width() * s_keySizePropotion);
    for (int i = 0; i < s_keypadRows; ++i) {
        for (int j = 0; j < s_keypadCols; ++j) {
            KeypadItem* item = new KeypadItem(j + s_keypadCols*i, QRectF(QPointF(x, y), keySize), this);
            connect(item, SIGNAL(itemPressed(KeypadItem&)), SLOT(keypadItemPressed(KeypadItem&)));
            connect(item, SIGNAL(itemSelected(KeypadItem&)), SLOT(keypadItemSelected(KeypadItem&)));
            m_buttons.append(item);
            x+= keySize.width();
        }
        x = m_keypadRect.left() + s_keypadXMargin;
        y+= keySize.height();
    }

    int specialKeyWidth = keySize.width() * s_speclialKeySizePropotion;
    for (int i = 0; i < s_keypadSpecialCols; ++i) {
        // special handling for space, it takes over the leftover place, in the middle
        QSizeF itemSize(specialKeyWidth, keySize.height());
        if (s_specialKeypadLine[i] == s_space) {
            int lo = m_keypadRect.right() - x - s_keypadXMargin - ((s_keypadSpecialCols - i - 1)*specialKeyWidth);
            itemSize.setWidth(lo);
        }
        KeypadItem* item = new KeypadItem(s_specialKeypadLine[i], QRectF(QPointF(x, y), itemSize), this);
        connect(item, SIGNAL(itemPressed(KeypadItem&)), SLOT(keypadItemPressed(KeypadItem&)));
        connect(item, SIGNAL(itemSelected(KeypadItem&)), SLOT(keypadItemSelected(KeypadItem&)));
        m_buttons.append(item);
        x+= itemSize.width();
    }
}

#include "KeypadWidget.moc"
