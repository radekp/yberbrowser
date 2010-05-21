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

#ifndef PopupView_h_
#define PopupView_h_

#include "TileSelectionViewBase.h"

class PannableViewport;
class PopupWidget;
class TileItem;
class Suggest;
class QTimer;
class QPixmap;

class PopupView : public TileSelectionViewBase {
    Q_OBJECT
public:
    PopupView(QGraphicsItem* parent = 0, QPixmap* bckg = 0, Qt::WindowFlags wFlags = 0);
    ~PopupView();

    void resizeEvent(QGraphicsSceneResizeEvent* event);

    void setFilterText(const QString& text);

Q_SIGNALS:
    void pageSelected(const QUrl&);

private Q_SLOTS:
    void tileItemActivated(TileItem*);
    void tileItemClosed(TileItem*);
    void tileItemEditingMode(TileItem*);
    void startSuggest();
    void populateSuggestion();

private:
    bool setupInAndOutAnimation(bool);
    void createViewItems();
    void destroyViewItems();

private:
    Suggest* m_suggest;
    QTimer* m_suggestTimer;
    PopupWidget* m_popupWidget;
    PannableViewport* m_pannableContainer;
    QString m_filterText;
};

#endif
