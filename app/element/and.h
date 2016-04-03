#ifndef AND_H
#define AND_H

#include "graphicelement.h"

class And : public GraphicElement {
public:
  explicit And( QGraphicsItem *parent = 0 );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::AND );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::GATE );
  }
  void updateLogic( );


};

#endif /* AND_H */
