#ifndef XOR_H
#define XOR_H

#include "graphicelement.h"

class Xor : public GraphicElement {
public:
  explicit Xor( QGraphicsItem *parent );
  virtual ~Xor( )override;

/* GraphicElement interface */
public:
  virtual ElementType elementType( )const override {
    return( ElementType::XOR );
  }
  virtual ElementGroup elementGroup( )const override {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* XOR_H */
