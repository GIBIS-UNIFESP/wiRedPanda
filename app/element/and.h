#ifndef AND_H
#define AND_H

#include "graphicelement.h"

class And : public GraphicElement {
public:
  explicit And( QGraphicsItem *parent = nullptr );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::AND );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::GATE );
  }

};

#endif /* AND_H */
