#ifndef JKFLIPFLOP_H
#define JKFLIPFLOP_H

#include "graphicelement.h"

class JKFlipFlop : public GraphicElement {
  bool lastClk;
  char lastJ;
  char lastK;
public:
  explicit JKFlipFlop( QGraphicsItem *parent = nullptr );
  virtual ~JKFlipFlop( ) override = default;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::JKFLIPFLOP );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( ) override;
};

#endif /* JKFLIPFLOP_H */
