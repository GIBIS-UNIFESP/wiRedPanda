#ifndef TFLIPFLOP_H
#define TFLIPFLOP_H

#include <graphicelement.h>

class TFlipFlop : public GraphicElement {
  bool lastClk;
public:
  explicit TFlipFlop( QGraphicsItem *parent = 0 );
  virtual ~TFlipFlop( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::TFLIPFLOP );
  }
  virtual void updatePorts( );
  void updateLogic( );
};

#endif /* TFLIPFLOP_H */
