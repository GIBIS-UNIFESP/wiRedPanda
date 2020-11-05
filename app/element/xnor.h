#ifndef XNOR_H
#define XNOR_H

#include "graphicelement.h"

class Xnor : public GraphicElement {
public:
  explicit Xnor( QGraphicsItem *parent = nullptr );
  virtual ~Xnor( ) override = default;

/* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::XNOR );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::GATE );
  }
};

#endif /* Xnor_H */