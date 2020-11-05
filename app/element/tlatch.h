#ifndef TLATCH_H
#define TLATCH_H

#include "graphicelement.h"

class TLatch : public GraphicElement {
public:
  explicit TLatch( QGraphicsItem *parent = nullptr );
  virtual ~TLatch( ) override = default;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::TLATCH );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( ) override;
};

#endif /* TLATCH_H */