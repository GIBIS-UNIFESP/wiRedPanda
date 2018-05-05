#ifndef NODE_H
#define NODE_H

#include "graphicelement.h"

class Node : public GraphicElement {
public:
  explicit Node( QGraphicsItem *parent = nullptr );
  virtual void updatePorts( )override;
  virtual ElementGroup elementGroup( )const override {
    return( ElementGroup::GATE );
  }
  /* GraphicElement interface */
public:
  ElementType elementType( ) const override;
};

#endif /* NODE_H */
