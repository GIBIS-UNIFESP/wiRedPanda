#ifndef XNOR_H
#define XNOR_H

#include "graphicelement.h"

class Xnor : public GraphicElement {
public:
  explicit Xnor(QGraphicsItem * parent);
  virtual ~Xnor();

// GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::XNOR;
  }
  void updateLogic();
};

#endif // Xnor_H
