#ifndef NOT_H
#define NOT_H

#include "graphicelement.h"

class Not : public GraphicElement {
public:
  explicit Not( QGraphicsItem *parent = nullptr);
  virtual ~Not() override = default;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override{
    return( ElementType::NOT );
  }
  virtual ElementGroup elementGroup( ) override{
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* NOT_H */
