#ifndef XOR_H
#define XOR_H

#include "graphicelement.h"

class Xor : public GraphicElement {
public:
  explicit Xor( QGraphicsItem *parent );
  virtual ~Xor( );

/* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::XOR );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* XOR_H */
