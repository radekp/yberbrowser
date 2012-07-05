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
#include "AutoSelectLineEdit_p.h"
#include "FontFactory.h"
#include "KeypadWidget.h"
#include "PopupView.h"
#include "ToolbarWidget.h"
#include "YberApplication.h"
#if USE_MEEGOTOUCH
#include <MApplicationPage>
#endif
#include <QUrl>
#ifdef QTOPIA
#include <QtopiaApplication>
#endif

const int s_clientItemMargin = 5;

/*! \class AutoSelectLineEdit input element (\QLineEdit) that selects
  its contents when focused in.

  Needed, because I did not find the feature from stock Qt widgets.
 */
AutoSelectLineEdit::AutoSelectLineEdit(QGraphicsItem* parent)
    : QGraphicsWidget(parent)
    , d(new AutoSelectLineEditPrivate(this))
    , m_virtualKeypad(0)
    , m_urlfilterPopup(0)
    , m_proxyWidget(new QGraphicsProxyWidget(this))
{
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    // create internal widgets
    m_proxyWidget->setWidget(d);
    m_proxyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // remove default background
    d->setFrame(false);
    d->setAttribute(Qt::WA_NoSystemBackground);
    d->setStyleSheet("background: transparent;");

    setFocusProxy(m_proxyWidget);

    // set font settings
    QPalette palette = d->palette();
    palette.setColor(QPalette::Text, Qt::white);
    d->setPalette(palette);
    d->setFont(FontFactory::instance()->medium());
}

AutoSelectLineEdit::~AutoSelectLineEdit()
{
    delete m_virtualKeypad;
    delete m_urlfilterPopup;
    delete d;
}

void AutoSelectLineEdit::setGeometry(const QRectF& rect)
{
    QGraphicsWidget::setGeometry(rect);
    m_proxyWidget->resize(rect.size());

#ifdef Q_OS_SYMBIAN
    // FIXME why does the text color change on orientation
    QPalette palette = d->palette();
    palette.setColor(QPalette::Text, Qt::white);
    d->setPalette(palette);
#endif
}

QString AutoSelectLineEdit::text()
{
    return d->url;
}

void AutoSelectLineEdit::setText(const QString& text)
{
    // FIXME: Move elsewhere.
    // When the user is editing, we don't want a loadFinished to override the current text.
    //if (d->hasFocus())
    //    return;
    d->url = text;
    d->adjustText();
}

int AutoSelectLineEdit::selectionStart() const
{
    return d->selectionStart();
}

void AutoSelectLineEdit::setCursorPosition(int pos)
{
    d->setCursorPosition(pos);
}

void AutoSelectLineEdit::setSelection(int start, int length)
{
    d->setSelection(start, length);
}

void AutoSelectLineEdit::setKeypadVisible(bool on)
{
#ifdef QTOPIA
    if(on)
        QtopiaApplication::showInputMethod();
    else
        QtopiaApplication::hideInputMethod();
    return;
#endif

    if ((on && m_virtualKeypad) || (!on && !m_virtualKeypad))
        return;

    if (on) {
        popupDismissed();
        createVirtualKeypad();
    } else {
        keypadDismissed();
    }
}

void AutoSelectLineEdit::newEditedText(const QString& newText)
{
    // this is a non-programatic text change (typing)
    keypadDismissed();
    // create home view and remove popupview when no text in the url field
    if (newText.isEmpty())
        popupDismissed();
    else {
        if (!m_urlfilterPopup)
            createUrlFilterPopup();
        m_urlfilterPopup->setFilterText(newText);
    }
    update();
    emit textEdited(newText);
}

void AutoSelectLineEdit::popupItemSelected(const QUrl& url)
{
    cleanupAndSendFinished(url.toString());
}

void AutoSelectLineEdit::popupDismissed()
{
    d->setFocus();
    delete m_urlfilterPopup;
    m_urlfilterPopup = 0;
    update();
}

void AutoSelectLineEdit::keypadCharEntered(char key)
{
    // FIXME: there must be a less hackish way to do it
    if (d->hasSelectedText())
        d->url.remove(d->selectionStart(), d->selectedText().size()); 
    setText(d->url + key);
    emit textEdited(text());
}

void AutoSelectLineEdit::keypadBackspace()
{
    // cant do d->backspace() as that goes through the typing codepath (apparently a programmatically change), 
    // removing the keypad
    QString newText(d->url);
    if (d->hasSelectedText())
        newText.remove(d->selectionStart(), d->selectedText().size()); 
    else if (!newText.isEmpty())
        newText.remove(newText.size() - 1, 1);
    setText(newText);
    emit textEdited(text());
}

void AutoSelectLineEdit::keypadEnter()
{
    cleanupAndSendFinished(text());
}

void AutoSelectLineEdit::keypadTextEntered(const QString& newText)
{
    if (d->hasSelectedText())
        d->url.remove(d->selectionStart(), d->selectedText().size()); 
    setText(d->url + newText);
    emit textEdited(text());
}

void AutoSelectLineEdit::keypadDismissed()
{
    d->setFocus();
    delete m_virtualKeypad;
    m_virtualKeypad = 0;
    update();
    emit keypadVisible(false);
}

void AutoSelectLineEdit::returnPressed()
{
    cleanupAndSendFinished(text());
}

bool AutoSelectLineEdit::realFocusEvent()
{
    return !m_urlfilterPopup && !m_virtualKeypad;
}

void AutoSelectLineEdit::createVirtualKeypad()
{
    QRectF r(parentView()->geometry());
    qDebug() << parentView() << parentView()->geometry();

    m_virtualKeypad = new KeypadWidget(parentView());
    m_virtualKeypad->setPos(r.topLeft());
    m_virtualKeypad->resize(r.size());
    m_virtualKeypad->appear(r.bottom());
    // FIXME: text() includes the suggested part too, so only a subset comes up initially
    m_virtualKeypad->updatePopup(text());

    connect(m_virtualKeypad, SIGNAL(charEntered(char)), SLOT(keypadCharEntered(char)));
    connect(m_virtualKeypad, SIGNAL(enter()), SLOT(keypadEnter()));
    connect(m_virtualKeypad, SIGNAL(backspace()), SLOT(keypadBackspace()));
    connect(m_virtualKeypad, SIGNAL(textEntered(const QString&)), SLOT(keypadTextEntered(const QString&)));
    connect(m_virtualKeypad, SIGNAL(dismissed()), SLOT(keypadDismissed()));
    connect(m_virtualKeypad, SIGNAL(pageSelected(const QUrl&)), this, SLOT(popupItemSelected(const QUrl&)));
    connect(this, SIGNAL(textEdited(const QString&)), m_virtualKeypad, SLOT(updatePopup(const QString&)));
    emit keypadVisible(true);
}

void AutoSelectLineEdit::createUrlFilterPopup()
{
    QRectF r(clientRect());
    m_urlfilterPopup = new PopupView(parentView());
    m_urlfilterPopup->setPos(r.topLeft());
    m_urlfilterPopup->resize(r.size());
    m_urlfilterPopup->appear();
    // only the nonselected text to not to include autocomplete in the filter 
    // (bugish as the user can very well do text selection too, sorry user)
    QString str(text());
    if (d->hasSelectedText())
        str.remove(d->selectionStart(), d->selectedText().size()); 
    m_urlfilterPopup->setFilterText(str);

    connect(m_urlfilterPopup, SIGNAL(pageSelected(const QUrl&)), this, SLOT(popupItemSelected(const QUrl&)));
    connect(m_urlfilterPopup, SIGNAL(viewDismissed()), this, SLOT(popupDismissed()));
}

QRectF AutoSelectLineEdit::clientRect() const
{
    // client items like keypad and popup should offset back to
    // the edge of the screen
    QRectF r(parentView()->geometry());
    r.moveLeft(r.left() - pos().x());
    r.moveTop(rect().bottom() + s_clientItemMargin);
    r.setHeight(r.height() - rect().height() - s_clientItemMargin);
    return r;
}

void AutoSelectLineEdit::cleanupAndSendFinished(const QString& text)
{
    popupDismissed();
    keypadDismissed();
    clearFocus();
    emit textEditingFinished(text);
}

QGraphicsWidget* AutoSelectLineEdit::parentView()
{
    return YberApplication::instance()->activeApplicationWindow()->currentPage();
}
