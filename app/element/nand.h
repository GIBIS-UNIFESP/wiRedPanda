#ifndef NAND_H
#define NAND_H

#include "graphicelement.h"

class Nand : public GraphicElement {
public:
  explicit Nand( QGraphicsItem *parent = nullptr );
  virtual ~Nand( ) override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::NAND );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::GATE );
  }
  void updateLogic( );
};

#endif /* NAND_H */
