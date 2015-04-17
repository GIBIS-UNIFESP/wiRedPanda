#ifndef XOR_H
#define XOR_H

#include "graphicelement.h"

class Xor : public GraphicElement{
public:
  Xor(QGraphicsItem * parent);
  ~Xor();

// GraphicElement interface
public:
void updateLogic();
};

#endif // XOR_H
