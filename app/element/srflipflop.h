#ifndef SRFLIPFLOP_H
#define SRFLIPFLOP_H

#include "graphicelement.h"

class SRFlipFlop : public GraphicElement {
  bool lastClk;

public:
  explicit SRFlipFlop( QGraphicsItem *parent = nullptr );
  virtual ~SRFlipFlop( )override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::SRFLIPFLOP );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( )override;
  void updateLogic( );
};

#endif /* SRFLIPFLOP_H */
