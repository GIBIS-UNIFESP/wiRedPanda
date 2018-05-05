#ifndef OR_H
#define OR_H

#include "graphicelement.h"

class Or : public GraphicElement {
public:
  explicit Or( QGraphicsItem *parent = 0 );
  virtual ~Or( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::OR );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::GATE );
  }
};

#endif /* OR_H */
