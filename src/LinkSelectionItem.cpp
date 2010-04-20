#include "LinkSelectionItem.h"
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>

namespace {
const qreal s_linkOpacity = 0.7;
const QColor s_linkRectangleColor(10, 10, 80);
const int s_appearAnimDuration = 350;
const int s_disappearAnimDuration = 650;
}

/*!
  \class LinkSelectionItem class responsible for implementing the link selection UI

  Responsibilities:
   * Defines the look and feel of an item selection
*/
LinkSelectionItem::LinkSelectionItem(QGraphicsItem* parent) 
    : QGraphicsRectItem(QRect(0, 0, 0, 0), parent) 
{
    setBrush(QBrush(s_linkRectangleColor));
    setOpacity(s_linkOpacity);
}

void LinkSelectionItem::appear(const QPointF& animStartPos, const QRectF& linkRect) 
{
    QGraphicsBlurEffect* blur = new QGraphicsBlurEffect();
    blur->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
    blur->setBlurRadius(15);
    setGraphicsEffect(blur);

    QPropertyAnimation* rectAnimation = new QPropertyAnimation(this, "rect");
    rectAnimation->setDuration(s_appearAnimDuration);

    rectAnimation->setStartValue(QRectF(animStartPos, QSize(3, 3)));
    rectAnimation->setEndValue(linkRect);    

    rectAnimation->setEasingCurve(QEasingCurve::OutExpo);

    QPropertyAnimation* opacityAnimation = new QPropertyAnimation(this, "opacity");
    opacityAnimation->setDuration(s_disappearAnimDuration);

    opacityAnimation->setStartValue(s_linkOpacity);
    opacityAnimation->setEndValue(0.0);

    opacityAnimation->setEasingCurve(QEasingCurve::InExpo);
    
    m_linkSelectiogroup.addAnimation(rectAnimation);
    m_linkSelectiogroup.addAnimation(opacityAnimation);
    m_linkSelectiogroup.start();
}


