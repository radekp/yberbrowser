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
#include <QPropertyAnimation>

const int s_toolbarSize = 48; // fixed 48 pixel
const int s_iconSize = 48; // fixed 48 pixel
const int s_thinToolbarHeight = 32;
#ifdef Q_OS_SYMBIAN
const int s_shrinkAnimTimeout = 200;
#else
const int s_shrinkAnimTimeout = 500;
#endif

ToolbarWidget::ToolbarWidget(QGraphicsItem* parent)
    : QGraphicsWidget(parent)
    , m_bookmarksIcon(QImage(":/data/icon/48x48/bookmarks_48.png"))
    , m_backIcon(QImage(":/data/icon/48x48/back_48.png"))
    , m_cancelIcon(QImage(":/data/icon/48x48/stop_48.png"))
    , m_progress(100)
    , m_urlEdit(new AutoSelectLineEdit(this))
{
#ifndef Q_OS_SYMBIAN
    //FIXME: cache mode is disabled, because it causes the icons not
    // to draw when animating back from the shrink state. This is due
    // to paint() being called for intermediate fractional size (like
    // 47.8), but not at the end size (like 48)
    // setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif
    connect(m_urlEdit, SIGNAL(textEdited(const QString&)), SLOT(textEdited(const QString&)));
    connect(m_urlEdit, SIGNAL(textEditingFinished(const QString&)), SLOT(textEditingFinished(const QString&)));
    connect(m_urlEdit, SIGNAL(focusChanged(bool)), SLOT(editorFocusChanged(bool)));
    connect(m_urlEdit, SIGNAL(keypadVisible(bool)), SLOT(keypadVisible(bool)));
    setZValue(1);
    QGradientStops stops;
    stops << QGradientStop(0.00, QColor(165, 165, 165, 220)) << QGradientStop(0.10, QColor(80, 80, 80, 225)) << QGradientStop(0.90, QColor(80, 80, 80, 225));
    for (int j = 0; j < stops.size(); ++j)
        m_bckgGradient.setColorAt(stops.at(j).first, stops.at(j).second);

    setMinimumHeight(s_thinToolbarHeight);
    setPreferredHeight(s_toolbarSize);
    setMaximumHeight(s_toolbarSize);
    resize(0, s_toolbarSize);
}

ToolbarWidget::~ToolbarWidget()
{
}

int ToolbarWidget::height()
{
    return s_toolbarSize;
}

void ToolbarWidget::setToolbarHeight(int height)
{
    QRectF r(geometry());
    r.setHeight(height);
    setGeometry(r);
}

int ToolbarWidget::toolbarHeight()
{
    return rect().height();
}

void ToolbarWidget::setTextIfUnfocused(const QString& text)
{
    // If the url edit is focused, it means that the user it in edit mode
    // and we, thus, do not want to change the text ie. on loadFinished().
    if (!m_urlEdit->hasFocus()) {
        m_urlEdit->setText(text);
        m_urlEdit->setCursorPosition(0);
    }
}

uint ToolbarWidget::progress()
{
    return m_progress;
}

void ToolbarWidget::setProgress(uint progress)
{
    m_progress = progress;
    if(progress < 100)
        update();
    else
        hide();
}

void ToolbarWidget::showKeypad()
{
    m_urlEdit->setKeypadVisible(true);
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

void ToolbarWidget::keypadVisible(bool on)
{
    animateToolbarMove(on);
}

void ToolbarWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRectF r(rect());

    static QRectF oldRect;
    bool geometryChanged = (r != oldRect);
    oldRect = r;
    if (geometryChanged) {
        m_bckgGradient.setStart(rect().bottomLeft());
        m_bckgGradient.setFinalStop(rect().topLeft());
    }

    // editor
    painter->setPen(QColor(80, 80, 80, 220));

    painter->setBrush(m_bckgGradient);
    painter->drawRoundedRect(r, 5, 5);

    int editorX = r.left() + s_iconSize;
    if (m_progress != 100) {
        QRectF pr(r);
        painter->setBrush(QColor(2, 2, 30, 120));
        int progressAreaWidth = r.width() - 2 * s_iconSize;
        pr.setLeft(editorX);
        pr.setRight(pr.left() + progressAreaWidth / 100 * m_progress);
        painter->drawRect(pr);
    }

    painter->setPen(QColor(20, 20, 20, 120));
    painter->setBrush(QColor(20, 20, 20, 120));

    // dont paint icons while in thin mode
    if (size().height() == s_toolbarSize) {
        // bookmark icon and bckg
        r.setRight(editorX);
        painter->drawRoundedRect(r, 3, 3);
        painter->drawImage(r.topLeft(), m_bookmarksIcon);

        // stop/cancel icon and bckg
        r.moveLeft(rect().right() - s_iconSize);
        painter->drawRoundedRect(r, 3, 3);
        painter->drawImage(r.topLeft(), m_progress == 100 ? m_backIcon : m_cancelIcon);
    }
}

void ToolbarWidget::mousePressEvent(QGraphicsSceneMouseEvent*)
{
}

void ToolbarWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QRectF r(rect());
    r.setRight(r.left() + s_iconSize);
    if (r.contains(event->pos())) {
        emit bookmarkPressed();
        return;
    }

    r.setLeft(rect().right() - s_iconSize);
    r.setRight(rect().right());

    if (r.contains(event->pos())) {
        if (m_progress == 100)
            emit backPressed();
        else
            emit cancelPressed();
        return;
    } 
}

void ToolbarWidget::animateToolbarMove(bool animateToSmall)
{
    QRectF tgt = geometry();
    tgt.setHeight(animateToSmall ? s_thinToolbarHeight : s_toolbarSize);

    QPropertyAnimation* moveAnim = new QPropertyAnimation(this, "geometry");
    moveAnim->setDuration(s_shrinkAnimTimeout);

    moveAnim->setStartValue(geometry());
    moveAnim->setEndValue(tgt);

    moveAnim->setEasingCurve(QEasingCurve::OutQuad);
    moveAnim->start();
}

/*!
 * \reimp
 * we need to catch resize because the layout of WebViewport depends
 * on the size of the toolbar. However, we have two modes:
 * 1) fixed to content
 * 2) fixed to viewport
 *
 * In mode 1, we could catch resizes from a custom QGraphicsLayout
 * In mode 2, we want to add the toolbar to the parent layout,
 * e.g BrowsingView layout. This is to prevent moving the toolbar
 * constantly as the content inside viewport moves (e.g. the
 * toolbar is not inside the viewport, but it affects the viewport
 * layout
 */
void ToolbarWidget::setGeometry(const QRectF& r)
{
    QGraphicsWidget::setGeometry(r);
    m_urlEdit->setGeometry(QRectF(QPointF(r.left() + s_iconSize, r.top()), QSizeF(r.width() - 2 * s_iconSize, r.height())));

    emit sizeUpdated();
}

bool ToolbarWidget::urlHasFocus()
{
    return m_urlEdit->hasFocus();
}
