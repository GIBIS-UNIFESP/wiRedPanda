#ifndef NAND_H
#define NAND_H

#include "graphicelement.h"

class Nand : public GraphicElement {
public:
  explicit Nand(QGraphicsItem *parent = 0);
  ~Nand();

  // GraphicElement interface
public:
  void updateLogic();
};

#endif // NAND_H

