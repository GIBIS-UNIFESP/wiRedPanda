#ifndef NOR_H
#define NOR_H

#include "graphicelement.h"

class Nor : public GraphicElement {
public:
  explicit Nor(QGraphicsItem * parent);
  ~Nor();

  // GraphicElement interface
public:
  void updateLogic();
};

#endif // NOR_H

