#ifndef NAND_H
#define NAND_H

#include "graphicelement.h"

class Nand : public GraphicElement {
public:
  explicit Nand( QGraphicsItem *parent = nullptr );
  virtual ~Nand( ) override = default;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::NAND );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::GATE );
  }
};

#endif /* NAND_H */