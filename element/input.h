#include "graphicelement.h"

#ifndef INPUT_H
#define INPUT_H


class Input : public GraphicElement {
public:
  Input(QGraphicsItem * parent);
  ~Input();

  // QGraphicsItem interface
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
};

#endif // INPUT_H
