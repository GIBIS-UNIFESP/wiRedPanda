#ifndef XOR_H
#define XOR_H

#include "graphicelement.h"

class Xor : public GraphicElement {
public:
  explicit Xor( QGraphicsItem *parent = nullptr );
  virtual ~Xor( ) override = default;

/* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::XOR );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::GATE );
  }
};

#endif /* XOR_H */
