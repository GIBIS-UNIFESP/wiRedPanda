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

  // GraphicElement interface
public:
  void updateLogic();

};

#endif // INPUTSWITCH_H
