#ifndef TFLIPFLOP_H
#define TFLIPFLOP_H

#include "graphicelement.h"

class TFlipFlop : public GraphicElement {
  bool lastClk;
  char lastT;
  char lastQ;
public:
  explicit TFlipFlop( QGraphicsItem *parent = 0 );
  virtual ~TFlipFlop( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::TFLIPFLOP );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( );
  void updateLogic( );
};

#endif /* TFLIPFLOP_H */
