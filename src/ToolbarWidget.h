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

#ifndef ToolbarWidget_h_
#define ToolbarWidget_h_

#include <QGraphicsWidget>
#include <QLinearGradient>

class PopupView;
class QImage;
class QStyleOptionGraphicsItem;
class QWidget;
class QGraphicsSceneMouseEvent;
class AutoSelectLineEdit;

class ToolbarWidget : public QGraphicsWidget {
    Q_OBJECT
    Q_PROPERTY(int toolbarHeight READ toolbarHeight WRITE setToolbarHeight)
public:
    ToolbarWidget(QGraphicsItem* parent);
    ~ToolbarWidget();

    static int height();
    void setToolbarHeight(int);
    int toolbarHeight();

    void setTextIfUnfocused(const QString& text);
    uint progress();
    void setProgress(uint progress);

    void showKeypad();

    void setGeometry(const QRectF&);

    bool urlHasFocus();

Q_SIGNALS:
    void bookmarkPressed();
    void cancelPressed();
    void backPressed();
    void urlEditingFinished(const QString& url);
    void urlEditorFocusChanged(bool);
    void sizeUpdated();

protected Q_SLOTS:
    void textEdited(const QString&);
    void textEditingFinished(const QString&);
    void editorFocusChanged(bool);

    void keypadVisible(bool);

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void animateToolbarMove(bool);

private:
    QImage m_bookmarksIcon;
    QImage m_backIcon;
    QImage m_cancelIcon;
    QImage m_keypadIcon;
    QString m_text;
    uint m_progress;
    bool m_editMode;
    AutoSelectLineEdit* m_urlEdit;
    QString m_lastEnteredText;
    QLinearGradient m_bckgGradient;
};

#endif
