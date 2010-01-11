#ifndef EventHelpers_h
#define EventHelpers_h

#include <QEvent>

class QGraphicsSceneMouseEvent;

void copyMouseEvent(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to);

void copyMouseEventPositions(const QGraphicsSceneMouseEvent* from, QGraphicsSceneMouseEvent* to);


#endif
