#ifndef NOR_H
#define NOR_H

#include "graphicelement.h"

class Nor : public GraphicElement {
public:
  explicit Nor( QGraphicsItem *parent );
  virtual ~Nor( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::NOR );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* NOR_H */
