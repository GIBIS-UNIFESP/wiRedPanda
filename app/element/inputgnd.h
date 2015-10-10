#ifndef INPUTGND_H
#define INPUTGND_H

#include "graphicelement.h"

class InputGnd : public GraphicElement {
public:
  explicit InputGnd(QGraphicsItem * parent);
  ~InputGnd();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::GND;
  }
  void updateLogic();
};

#endif // INPUTGND_H

