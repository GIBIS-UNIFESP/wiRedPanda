#ifndef TFLIPFLOP_H
#define TFLIPFLOP_H

#include <graphicelement.h>

class TFlipFlop : public GraphicElement {
public:
  explicit TFlipFlop(QGraphicsItem * parent);
  ~TFlipFlop();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::TFLIPFLOP;
  }
  virtual void updatePorts();
  void updateLogic();
};

#endif // TFLIPFLOP_H
