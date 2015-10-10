#ifndef OR_H
#define OR_H

#include "graphicelement.h"

class Or : public GraphicElement {
public:
  explicit Or(QGraphicsItem * parent);
  ~Or();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::OR;
  }
  void updateLogic();
};

#endif // OR_H
