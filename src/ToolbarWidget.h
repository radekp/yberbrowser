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

#include <QGraphicsRectItem>
#include <QLinearGradient>

class PopupView;
class QImage;
class QStyleOptionGraphicsItem;
class QWidget;
class QGraphicsSceneMouseEvent;
class AutoSelectLineEdit;
class QGraphicsProxyWidget;

class ToolbarWidget : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    ToolbarWidget(QGraphicsItem* parent);
    ~ToolbarWidget();

    // FIXME: 
    static int height();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void setEditMode(bool on);
    void setText(const QString& text);
    void setProgress(uint progress);
    QString text();

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

Q_SIGNALS:
    void bookmarkPressed();    
    void cancelPressed();    
    void backPressed();    
    void urlTextChanged(const QString& newText);
    void urlEditingFinished(const QString& url);
    void urlEditorFocusChanged(bool);    

protected Q_SLOTS:
    void textEdited(const QString&);
    void textEditingFinished();
    void editorFocusChanged(bool);
    void popupItemSelected(const QUrl&);
    void popupDismissed();

private:
    void createPopup();

private:
    QImage* m_bookmarksIcon;
    QImage* m_backIcon;
    QImage* m_cancelIcon;
    QString m_text;
    uint m_progress;
    bool m_editMode;
    QGraphicsProxyWidget* m_urlProxyWidget;
    AutoSelectLineEdit* m_urlEdit;
    QString m_lastEnteredText;
    PopupView* m_urlfilterPopup;
    QLinearGradient m_bckgGradient;
    bool m_gradientSet;
};

#endif
