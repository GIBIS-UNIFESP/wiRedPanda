#ifndef NAND_H
#define NAND_H

#include "graphicelement.h"

class Nand : public GraphicElement {
public:
  explicit Nand(QGraphicsItem *parent = 0);
  virtual ~Nand();

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::NAND;
  }
  void updateLogic();
};

#endif // NAND_H

