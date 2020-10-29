#ifndef DLATCH_H
#define DLATCH_H

#include "graphicelement.h"

class DLatch : public GraphicElement {
public:
  explicit DLatch( QGraphicsItem *parent = nullptr );
  virtual ~DLatch( ) override = default;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::DLATCH );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( ) override;
  void updateLogic( );
};

#endif /* DLATCH_H */