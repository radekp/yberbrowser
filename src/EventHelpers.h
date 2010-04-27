#ifndef EventHelpers_h
#define EventHelpers_h

#include <QEvent>
#include <QGraphicsItem>

class QGraphicsSceneMouseEvent;

void copyMouseEvent(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to);

void copyMouseEventPositions(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to);

/*!
  Copies common properties of mouse/wheel event to new event. Maps the coordinates from the event
  to the coordinate space of the receiving item

  This is very errorprone and should be replaced.
  \internal
 */
template<typename EventType>
void mapEventCommonProperties(QGraphicsItem* parent, const EventType* event, EventType& mappedEvent)
{
    // lifted form qmlgraphicsflickable.cpp, LGPL Nokia

    QRectF parentRect = parent->mapToScene(parent->boundingRect()).boundingRect();

    mappedEvent.setAccepted(false);
    mappedEvent.setPos(parent->mapFromScene(event->scenePos()));
    mappedEvent.setScenePos(event->scenePos());
    mappedEvent.setScreenPos(event->screenPos());
    mappedEvent.setModifiers(event->modifiers());
    mappedEvent.setButtons(event->buttons());
}



#endif
