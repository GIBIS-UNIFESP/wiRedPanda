#ifndef INPUTGND_H
#define INPUTGND_H

#include "graphicelement.h"

class InputGnd : public GraphicElement {
public:
  explicit InputGnd(QGraphicsItem * parent);
  ~InputGnd();
  // GraphicElement interface
public:
  void updateLogic();
};

#endif // INPUTGND_H

