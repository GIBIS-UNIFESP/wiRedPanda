#ifndef DFLIPFLOP_H
#define DFLIPFLOP_H

#include <graphicelement.h>

class DFlipFlop : public GraphicElement {
public:
  explicit DFlipFlop(QGraphicsItem * parent);
  ~DFlipFlop();

    // GraphicElement interface
  public:
    virtual void updatePorts();
};

#endif // DFLIPFLOP_H
