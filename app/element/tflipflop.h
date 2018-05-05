#ifndef TFLIPFLOP_H
#define TFLIPFLOP_H

#include "graphicelement.h"

class TFlipFlop : public GraphicElement {
  bool lastClk;
  char lastT;
  char lastQ;
public:
  explicit TFlipFlop( QGraphicsItem *parent = nullptr );
  virtual ~TFlipFlop( )override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( )const override {
    return( ElementType::TFLIPFLOP );
  }
  virtual ElementGroup elementGroup( )const override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( )override;
  void updateLogic( );
};

#endif /* TFLIPFLOP_H */
