#ifndef XNOR_H
#define XNOR_H

#include "graphicelement.h"

class Xnor : public GraphicElement {
public:
  explicit Xnor(QGraphicsItem * parent);
  ~Xnor();

// GraphicElement interface
public:
  void updateLogic();
};

#endif // Xnor_H
