#ifndef JKFLIPFLOP_H
#define JKFLIPFLOP_H

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement {
  bool lastClk;
public:
  explicit JKFlipFlop(QGraphicsItem * parent = 0);

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::JKFLIPFLOP;
  }
  virtual void updatePorts();
  void updateLogic();
};

#endif // JKFLIPFLOP_H
