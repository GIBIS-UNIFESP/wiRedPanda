#ifndef INPUTGND_H
#define INPUTGND_H

#include "graphicelement.h"

class InputGnd : public GraphicElement {
public:
  explicit InputGnd( QGraphicsItem *parent = nullptr );
  virtual ~InputGnd( )override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::GND );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::INPUT );
  }
  void updateLogic( );
};

#endif /* INPUTGND_H */
