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
#include "AutoSelectLineEdit.h"
#include "Settings.h"
#include "HistoryStore.h"
#include "KeypadWidget.h"

#include <QUrl>
#include <QImage>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

const int s_toolbarSize = 48; // fixed 48 pixel
const int s_iconXMargin = 10;

ToolbarWidget::ToolbarWidget(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , m_bookmarksIcon(QImage(":/data/icon/48x48/bookmarks_48.png"))
    , m_backIcon(QImage(":/data/icon/48x48/back_48.png"))
    , m_cancelIcon(QImage(":/data/icon/48x48/stop_48.png"))
    , m_progress(0)
    , m_urlEdit(new AutoSelectLineEdit(this))
{
#ifndef Q_OS_SYMBIAN
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif    
    connect(m_urlEdit, SIGNAL(textEdited(const QString&)), SLOT(textEdited(const QString&)));
    connect(m_urlEdit, SIGNAL(textEditingFinished(const QString&)), SLOT(textEditingFinished(const QString&)));
    connect(m_urlEdit, SIGNAL(focusChanged(bool)), SLOT(editorFocusChanged(bool)));
    setZValue(1);
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(165, 165, 165, 220)) << QGradientStop(0.10, QColor(80, 80, 80, 225)) << QGradientStop(0.90, QColor(80, 80, 80, 225));
    for (int j = 0; j < stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);
}

ToolbarWidget::~ToolbarWidget()
{
}

int ToolbarWidget::height()
{
    return s_toolbarSize;
}

void ToolbarWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    static QRectF oldRect;
    QRectF r(rect());
    bool geometryChanged = (r != oldRect);
    oldRect = r;

    if (geometryChanged) {
        m_urlEdit->setGeometry(QRectF(QPointF(r.left() + s_toolbarSize, r.top()), QSizeF(r.width() - 2 * s_toolbarSize, r.height())));

        m_bckgGradient.setStart(rect().bottomLeft());
        m_bckgGradient.setFinalStop(rect().topLeft());
    }

    // editor
    painter->setPen(QColor(80, 80, 80, 220));

    painter->setBrush(m_bckgGradient);
    painter->drawRect(r);

    int editorX = r.left() + s_toolbarSize;
    if (m_progress > 0) {
        QRectF pr(r);
        painter->setBrush(QColor(2, 2, 30, 120));
        int progressAreaWidth = r.width() - 2 * s_toolbarSize;
        pr.setLeft(editorX);
        pr.setRight(pr.left() + progressAreaWidth / 100 * m_progress);
        painter->drawRect(pr);
    }

    painter->setPen(QColor(20, 20, 20, 120));
    painter->setBrush(QColor(20, 20, 20, 120));

    // bookmark icon and bckg
    r.setRight(editorX);
    painter->drawRect(r);
    painter->drawImage(r.topLeft(), m_bookmarksIcon);

    // stop/cancel icon and bckg
    r.moveLeft(rect().right() - s_toolbarSize);
    painter->drawRect(r);
    painter->drawImage(r.topLeft(), m_progress > 0 ? m_cancelIcon : m_backIcon);
}

void ToolbarWidget::setTextIfUnfocused(const QString& text)
{
    // If the url edit is focused, it means that the user it in edit mode
    // and we, thus, do not want to change the text ie. on loadFinished().
    if (!m_urlEdit->hasFocus())
        m_urlEdit->setText(text);
}

void ToolbarWidget::setProgress(uint progress)
{
    m_progress = progress;
    update();
}

void ToolbarWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
}

void ToolbarWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QRectF r(rect());
    r.setRight(r.left() + s_toolbarSize);
    if (r.contains(event->pos())) {
        emit bookmarkPressed();
        return;
    }

    r.setLeft(rect().right() - s_toolbarSize);
    r.setRight(rect().right());

    if (r.contains(event->pos())) {
        if (m_progress == 0)
            emit backPressed();
        else
            emit cancelPressed();
        return;
    } 
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
}

void ToolbarWidget::textEditingFinished(const QString& text)
{
    m_lastEnteredText = QString();
    emit urlEditingFinished(text);
}

void ToolbarWidget::editorFocusChanged(bool focused)
{
    emit urlEditorFocusChanged(focused);
}


