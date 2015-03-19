#ifndef INPUTSWITCH_H
#define INPUTSWITCH_H

#include "graphicelement.h"

class InputSwitch : public GraphicElement {
public:
  explicit InputSwitch(QGraphicsItem * parent);
  ~InputSwitch();

  bool on;
    // QGraphicsItem interface
  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
};

#endif // INPUTSWITCH_H
