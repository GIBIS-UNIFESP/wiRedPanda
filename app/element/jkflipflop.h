#ifndef JKFLIPFLOP_H
#define JKFLIPFLOP_H

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement {
  bool lastClk;
  char lastJ;
  char lastK;
public:
  explicit JKFlipFlop( QGraphicsItem *parent = nullptr );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::JKFLIPFLOP );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( ) override;
  void updateLogic( );
};

#endif /* JKFLIPFLOP_H */
