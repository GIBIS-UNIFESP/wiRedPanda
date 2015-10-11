#ifndef NOT_H
#define NOT_H

#include "graphicelement.h"

class Not : public GraphicElement {
public:
  explicit Not(QGraphicsItem * parent);
  virtual ~Not();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::NOT;
  }
  void updateLogic();
};

#endif // NOT_H

