#ifndef JKFLIPFLOP_H
#define JKFLIPFLOP_H

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement {
  bool lastClk;
public:
  explicit JKFlipFlop(QGraphicsItem * parent);
  ~JKFlipFlop();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::JKFLIPFLOP;
  }
  virtual void updatePorts();
  void updateLogic();
};

#endif // JKFLIPFLOP_H
