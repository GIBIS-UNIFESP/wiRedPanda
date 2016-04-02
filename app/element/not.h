#ifndef NOT_H
#define NOT_H

#include "graphicelement.h"

class Not : public GraphicElement {
public:
  explicit Not( QGraphicsItem *parent );
  virtual ~Not( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::NOT );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* NOT_H */
