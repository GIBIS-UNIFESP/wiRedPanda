#ifndef OR_H
#define OR_H

#include "graphicelement.h"

class Or : public GraphicElement {
public:
  explicit Or( QGraphicsItem *parent = nullptr );
  virtual ~Or( ) override = default;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::OR );
  }
  virtual ElementGroup elementGroup( )override {
    return( ElementGroup::GATE );
  }
};

#endif /* OR_H */
