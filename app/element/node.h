#ifndef NODE_H
#define NODE_H

#include <graphicelement.h>

class Node : public GraphicElement {
public:
  explicit Node( QGraphicsItem *parent = 0 );
  virtual void updatePorts( );
  virtual void updateLogic( );

  // GraphicElement interface
public:
  ElementType elementType();
};

#endif /* NODE_H */
