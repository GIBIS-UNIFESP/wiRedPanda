#ifndef NOT_H
#define NOT_H

#include "graphicelement.h"

class Not : public GraphicElement {
public:
  explicit Not( QGraphicsItem *parent );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::NOT );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* NOT_H */
