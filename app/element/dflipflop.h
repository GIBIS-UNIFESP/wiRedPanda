#ifndef DFLIPFLOP_H
#define DFLIPFLOP_H

#include "graphicelement.h"

class DFlipFlop : public GraphicElement {
  bool lastClk;
  bool lastValue;

public:
  explicit DFlipFlop( QGraphicsItem *parent = nullptr );
  virtual ~DFlipFlop( )override;

  // GraphicElement interface
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::DFLIPFLOP );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( )override;

};

#endif // DFLIPFLOP_H
