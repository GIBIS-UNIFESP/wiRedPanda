#ifndef DFLIPFLOP_H
#define DFLIPFLOP_H

#include <graphicelement.h>

class DFlipFlop : public GraphicElement {
  bool lastClk;
  bool lastValue;

public:
  explicit DFlipFlop(QGraphicsItem * parent = 0);
  virtual ~DFlipFlop();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::DFLIPFLOP;
  }
  virtual void updatePorts();
  void updateLogic();

};

#endif // DFLIPFLOP_H

