#ifndef AND_H
#define AND_H

#include "graphicelement.h"

class And : public GraphicElement {
public:
  explicit And(QGraphicsItem *parent = 0);
  virtual ~And();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::AND;
  }
  void updateLogic();
};

#endif // AND_H
