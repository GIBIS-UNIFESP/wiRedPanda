#ifndef NOR_H
#define NOR_H

#include "graphicelement.h"

class Nor : public GraphicElement {
public:
  explicit Nor(QGraphicsItem * parent);
  ~Nor();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::NOR;
  }
  void updateLogic();
};

#endif // NOR_H

