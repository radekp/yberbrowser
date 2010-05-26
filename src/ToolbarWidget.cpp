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

#include "ToolbarWidget.h"
#include <QUrl>
#include <QImage>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

#include "AutoSelectLineEdit.h"
#include "PopupView.h"
#include "Settings.h"
#include "HistoryStore.h"

const int s_toolbarHeight = 56; // fixed 48 pixel icons and 4+4 margin
const int s_iconXMargin = 10;
const int s_toolbarIconWidth = 48;
const int s_toolbarIconMargin = 4;

ToolbarWidget::ToolbarWidget(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , m_bookmarksIcon(new QImage(":/data/icon/48x48/bookmarks_48.png"))
    , m_backIcon(new QImage(":/data/icon/48x48/back_48.png"))
    , m_cancelIcon(new QImage(":/data/icon/48x48/stop_48.png"))
    , m_progress(0)
    , m_editMode(false)
    , m_urlProxyWidget(0)
    , m_urlEdit(new AutoSelectLineEdit(0))
    , m_urlfilterPopup(0)
    , m_gradientSet(false)
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    connect(m_urlEdit, SIGNAL(textEdited(const QString&)), SLOT(textEdited(const QString&)));
    connect(m_urlEdit, SIGNAL(returnPressed()), SLOT(textEditingFinished()));
    connect(m_urlEdit, SIGNAL(focusChanged(bool)), SLOT(editorFocusChanged(bool)));
    setZValue(1);
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(165, 165, 165, 220)) << QGradientStop(0.10, QColor(80, 80, 80, 225)) << QGradientStop(0.90, QColor(80, 80, 80, 225));
    for (int j=0; j<stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);
}

ToolbarWidget::~ToolbarWidget()
{
}

int ToolbarWidget::height()
{
    return s_toolbarHeight;
}

void ToolbarWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF r(rect());

    // FIXME: do some kind of resize listening
    if (!m_gradientSet) {
        m_bckgGradient.setStart(rect().topLeft());
        m_bckgGradient.setFinalStop(rect().bottomLeft());
        m_gradientSet = true;
    }

    // editor
    painter->setPen(QColor(80, 80, 80, 220));

    painter->setBrush(m_bckgGradient);
    painter->drawRect(r);

    int editorX = r.left() + s_toolbarIconWidth + 2*s_toolbarIconMargin;
    if (m_progress > 0) {
        QRectF pr(r);
        painter->setBrush(QColor(2, 2, 30, 120));
        int progressAreaWidth = r.width() - 2*(s_toolbarIconWidth + 2*s_toolbarIconMargin);
        pr.setLeft(editorX);
        pr.setRight(pr.left() + progressAreaWidth / 100 * m_progress);
        painter->drawRect(pr);
    }

    painter->setPen(QColor(20, 20, 20, 120));
    painter->setBrush(QColor(20, 20, 20, 120));

    // bookmark icon and bckg
    r.setRight(editorX);
    int iconY = r.top() + s_toolbarIconMargin;
    painter->drawRect(r);
    painter->drawImage(QPointF(r.left() + s_toolbarIconMargin, iconY), *m_bookmarksIcon);

    // stop/cancel icon and bckg
    r.moveLeft(rect().right() - (s_toolbarIconWidth + 2*s_toolbarIconMargin));
    painter->drawRect(r);
    painter->drawImage(QPointF(rect().right() - (s_toolbarIconWidth + s_toolbarIconMargin), iconY), m_progress > 0 ? *m_cancelIcon : *m_backIcon);
    
    QFont f("Nokia Sans", 18);
    painter->setPen(QColor(Qt::white));
    painter->setFont(f);
    int texMargin = s_toolbarIconWidth + 3*s_toolbarIconMargin;
    r = rect(); 
    r.adjust(texMargin, 0, -texMargin, 0);

    // FIXME find out resize event and do the elideright there
    if (m_text.isEmpty())
        m_text = QFontMetrics(f).elidedText(m_urlEdit->text(), Qt::ElideRight, rect().width() - (2*s_toolbarIconWidth + 4*s_toolbarIconMargin));
    painter->drawText(r, Qt::AlignLeft|Qt::AlignVCenter, m_text); 
}

void ToolbarWidget::setText(const QString& text)
{
    m_text = QFontMetrics(QFont("Nokia Sans", 18)).elidedText(text, Qt::ElideRight, rect().width() - (2*s_toolbarIconWidth + 4*s_toolbarIconMargin));
    m_urlEdit->setText(text);
    update();
}

void ToolbarWidget::setEditMode(bool on)
{
    // dont close when editor looses focus while urlpopup on.
    if (on == m_editMode || m_urlfilterPopup)
        return;

    if (on) {
        if (!m_urlProxyWidget)
            m_urlProxyWidget = scene()->addWidget(m_urlEdit);

        QRectF r(rect());
        int buttonWidth = s_toolbarIconWidth + 2*s_toolbarIconMargin;
        m_urlProxyWidget->setGeometry(QRectF(QPointF(r.left() + buttonWidth, r.top()), QSizeF(r.width() - 2*buttonWidth, r.height())));
        m_urlEdit->show();
        m_urlEdit->setFocus(Qt::MouseFocusReason);
    } else {
        setText(m_urlEdit->text());
        m_urlEdit->hide();
    }
    m_editMode = on;
    update();
}

void ToolbarWidget::setProgress(uint progress)
{
    m_progress = progress;
    update();
}

QString ToolbarWidget::text() 
{ 
    return m_urlEdit->text(); 
}

void ToolbarWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
}

void ToolbarWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QRectF r(rect());
    int toolbarButtonWidth = s_toolbarIconWidth + 2*s_toolbarIconMargin;
    r.setRight(r.left() + toolbarButtonWidth);
    if (r.contains(event->pos())) {
        emit bookmarkPressed();
        return;
    } 

    r.setLeft(r.right());
    r.setRight(rect().right() - toolbarButtonWidth);

    if (!m_editMode && r.contains(event->pos()))
        emit urlEditorFocusChanged(true);
    else if (m_progress == 0)
        emit backPressed();
    else
        emit cancelPressed();
}

void ToolbarWidget::textEdited(const QString& newText)
{
    if (!m_urlEdit->isVisible() || !Settings::instance()->autoCompleteEnabled())
        return;

    QString text = newText;
    if (m_urlEdit->selectionStart() > -1)
        text = newText.left(m_urlEdit->selectionStart());
    // autocomplete only when adding text, not when deleting or backspacing
    if (text.size() > m_lastEnteredText.size()) {
        // todo: make it async
        QString match = HistoryStore::instance()->match(text);
        if (!match.isEmpty()) {
            m_urlEdit->setText(match);
            m_urlEdit->setCursorPosition(text.size());
            m_urlEdit->setSelection(text.size(), match.size() - text.size());
        }
    }
    m_lastEnteredText = newText;

    // create home view and remove popupview when no text in the url field
    if (newText.isEmpty())
        popupDismissed();
    else {
        if (!m_urlfilterPopup)
            createPopup();
        m_urlfilterPopup->setFilterText(newText);
    }
    emit urlTextChanged(newText);
}

void ToolbarWidget::textEditingFinished()
{
    m_lastEnteredText = QString();
    popupDismissed();
    emit urlEditingFinished(m_urlEdit->text());
}

void ToolbarWidget::editorFocusChanged(bool focused)
{
    if (!focused)
        urlEditorFocusChanged(false);
}

void ToolbarWidget::popupItemSelected(const QUrl& url)
{
    m_urlEdit->setText(url.toString());
    textEditingFinished();
}

void ToolbarWidget::popupDismissed()
{
    delete m_urlfilterPopup;
    m_urlfilterPopup = 0;
    setEditMode(false);
}

void ToolbarWidget::createPopup()
{
    if (m_urlfilterPopup)
        return;
    m_urlfilterPopup = new PopupView(this);
    connect(m_urlfilterPopup, SIGNAL(pageSelected(const QUrl&)), this, SLOT(popupItemSelected(const QUrl&)));
    connect(m_urlfilterPopup, SIGNAL(viewDismissed()), this, SLOT(popupDismissed()));
    m_urlfilterPopup->resize(parentWidget()->size().width(), parentWidget()->size().height() - rect().height());
    m_urlfilterPopup->appear();
}

