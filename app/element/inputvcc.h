#ifndef INPUTVCC_H
#define INPUTVCC_H

#include "graphicelement.h"

class InputVcc : public GraphicElement {
public:
  explicit InputVcc( QGraphicsItem *parent = 0 );
  virtual ~InputVcc( );
  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::VCC );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::INPUT );
  }
  void updateLogic( );
};

#endif /* INPUTVCC_H */
