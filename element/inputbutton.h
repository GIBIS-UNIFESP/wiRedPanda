#include "graphicelement.h"

#ifndef INPUT_H
#define INPUT_H


class InputButton : public GraphicElement {
public:
  InputButton(QGraphicsItem * parent);
  ~InputButton();

  // QGraphicsItem interface
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
};

#endif // INPUT_H
