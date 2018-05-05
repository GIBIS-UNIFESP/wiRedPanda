#ifndef XNOR_H
#define XNOR_H

#include "graphicelement.h"

class Xnor : public GraphicElement {
public:
  explicit Xnor( QGraphicsItem *parent );
  virtual ~Xnor( )override;

/* GraphicElement interface */
public:
  virtual ElementType elementType( )const override {
    return( ElementType::XNOR );
  }
  virtual ElementGroup elementGroup( )const override {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* Xnor_H */
