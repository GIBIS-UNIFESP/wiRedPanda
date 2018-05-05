#ifndef NOR_H
#define NOR_H

#include "graphicelement.h"

class Nor : public GraphicElement {
public:
  explicit Nor( QGraphicsItem *parent );
  virtual ~Nor( )override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::NOR );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* NOR_H */
