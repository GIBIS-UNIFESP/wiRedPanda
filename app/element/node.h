#ifndef NODE_H
#define NODE_H

#include "graphicelement.h"

class Node : public GraphicElement {
public:
  explicit Node( QGraphicsItem *parent = nullptr );
  virtual ~Node( ) override = default;

  virtual void updatePorts( ) override;
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::GATE );
  }
  /* GraphicElement interface */
public:
  ElementType elementType( ) override;
};

#endif /* NODE_H */