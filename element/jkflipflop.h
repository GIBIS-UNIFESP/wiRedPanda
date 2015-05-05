#ifndef JKFLIPFLOP_H
#define JKFLIPFLOP_H

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement {
public:
  explicit JKFlipFlop(QGraphicsItem * parent);
  ~JKFlipFlop();

  // GraphicElement interface
public:
  virtual void updatePorts();
};

#endif // JKFLIPFLOP_H
