#ifndef AND_H
#define AND_H

#include "graphicelement.h"

class And : public GraphicElement {
public:
  explicit And(QGraphicsItem *parent = 0);
  ~And();


  // GraphicElement interface
public:
  void updateLogic();
};

#endif // AND_H
