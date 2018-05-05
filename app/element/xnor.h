#ifndef XNOR_H
#define XNOR_H

#include "graphicelement.h"

class Xnor : public GraphicElement {
public:
  explicit Xnor( QGraphicsItem *parent );
  virtual ~Xnor( );

/* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::XNOR );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* Xnor_H */
