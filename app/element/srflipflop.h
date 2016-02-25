#ifndef SRFLIPFLOP_H
#define SRFLIPFLOP_H

#include <graphicelement.h>

class SRFlipFlop : public GraphicElement {
  bool lastClk;

public:
  explicit SRFlipFlop( QGraphicsItem *parent = 0 );
  virtual ~SRFlipFlop( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::SRFLIPFLOP );
  }
  virtual void updatePorts( );
  void updateLogic( );
};

#endif /* SRFLIPFLOP_H */
