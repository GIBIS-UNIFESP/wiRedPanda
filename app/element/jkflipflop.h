#ifndef JKFLIPFLOP_H
#define JKFLIPFLOP_H

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement {
  bool lastClk;
  char lastJ;
  char lastK;
public:
  explicit JKFlipFlop( QGraphicsItem *parent = 0 );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::JKFLIPFLOP );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( );
  void updateLogic( );
};

#endif /* JKFLIPFLOP_H */
