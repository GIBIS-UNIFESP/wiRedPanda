#ifndef INPUTVCC_H
#define INPUTVCC_H

#include "graphicelement.h"

class InputVcc : public GraphicElement {
public:
  explicit InputVcc( QGraphicsItem *parent = nullptr );
  virtual ~InputVcc( )override;
  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override{
    return( ElementType::VCC );
  }
  virtual ElementGroup elementGroup( ) const override{
    return( ElementGroup::INPUT );
  }
  void updateLogic( );
};

#endif /* INPUTVCC_H */
