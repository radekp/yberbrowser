#include <QGraphicsSceneMouseEvent>

#include "EventHelpers.h"


void copyMouseEvent(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to)
{
    for (int i = 0x1; i <= 0x10; i <<= 1) {
        if (from->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            to->setButtonDownPos(button, from->buttonDownPos(button));
            to->setButtonDownScenePos(button, from->buttonDownScenePos(button));
            to->setButtonDownScreenPos(button, from->buttonDownScreenPos(button));
        }
    }
    to->setAccepted(from->isAccepted());
    to->setButtons(from->buttons());
    to->setButton(from->button());
    to->setModifiers(from->modifiers());
    to->setWidget(from->widget());

    copyMouseEventPositions(from, to);
}

void copyMouseEventPositions(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to)
{
    to->setPos(from->pos());
    to->setScenePos(from->scenePos());
    to->setScreenPos(from->screenPos());
    to->setLastPos(from->lastPos());
    to->setLastScenePos(from->lastScenePos());
    to->setLastScreenPos(from->lastScreenPos());
}

