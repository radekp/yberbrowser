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

#ifndef WebViewport_h_
#define WebViewport_h_

#include "yberconfig.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QTimer>
#include "PannableViewport.h"

#include "CommonGestureRecognizer.h"

//#define ENABLE_LINK_SELECTION_VISUAL_DEBUG

class WebViewportItem;
class WebView;
class LinkSelectionItem;
class QGraphicsSceneMouseEvent;
#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
class QGraphicsRectItem;
class QGraphicsEllipseItem;
#endif

class WebViewport : public PannableViewport, private CommonGestureConsumer
{
    Q_OBJECT
public:
    WebViewport(QGraphicsItem* parent = 0);
    ~WebViewport();

    void startZoomAnimToItemHotspot(const QPointF& hotspot,  const QPointF& viewTargetHotspot, qreal scale);
    void setWebView(WebView* webview);
    WebViewportItem* viewportItem() const { return m_viewportWidget; }

public Q_SLOTS:
    void reset();

protected:
    bool sceneEventFilter(QGraphicsItem*, QEvent*);
    void wheelEvent(QGraphicsSceneWheelEvent*);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);
    void resizeEvent(QGraphicsSceneResizeEvent* event);

    void mousePressEventFromChild(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEventFromChild(QGraphicsSceneMouseEvent * event);
    void mouseDoubleClickEventFromChild(QGraphicsSceneMouseEvent * event);
    void adjustClickPosition(QPointF& pos);
    bool processMaemo5ZoomKeys(QKeyEvent* event);


 protected Q_SLOTS:
    void contentsSizeChangeCausedResize();
    void startLinkSelection();
    void enableBackingStoreUpdates();
    void zoomRectForPointReceived(const QPointF&, const QRectF&);

 private:
    void resetZoomAnim();
    void wheelEventFromChild(QGraphicsSceneWheelEvent *event);
    bool mouseEventFromChild(QGraphicsSceneMouseEvent *event);
    bool isZoomedIn() const;

    enum PanningState {
        Inactive,
        Pushing
    };

    void setPannedWidgetGeometry(const QRectF& r);
    void startPannedWidgetGeomAnim(const QPointF& pos, const QSizeF& size);
    void stopPannedWidgetGeomAnim();
    QRectF adjustRectForPannedWidgetGeometry(const QRectF&);
    void transferAnimStateToView();
    void updateViewportItemSizeIfDimensionPreserved();
    void updateViewportRange();

 private Q_SLOTS:
    void webPanningStarted();
    void webPanningStopped();
    void geomAnimStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State);

 private:
    WebViewportItem* m_viewportWidget;
    PanningState m_panningState;
    CommonGestureRecognizer m_recognizer;

    QEvent* m_selfSentEvent;

    QTimer m_backingStoreUpdateEnableTimer;
    LinkSelectionItem* m_linkSelectionItem; 
    QGraphicsSceneMouseEvent* m_delayedMouseReleaseEvent;

    QParallelAnimationGroup m_geomAnim;
    QPropertyAnimation m_posAnim;
    QPropertyAnimation m_sizeAnim;
    QRectF m_geomAnimEndValue;

#if defined(ENABLE_LINK_SELECTION_VISUAL_DEBUG)
    QGraphicsRectItem* m_searchRectItem;
    QGraphicsEllipseItem* m_clickablePointItem;
#endif
};


#endif
