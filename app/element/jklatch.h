#ifndef JKLATCH_H
#define JKLATCH_H

#include "graphicelement.h"

class JKLatch : public GraphicElement {
public:
  explicit JKLatch( QGraphicsItem *parent = nullptr );
  virtual ~JKLatch( )override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override {
    return( ElementType::JKLATCH );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( )override;
  void updateLogic( );
};

#endif /* JKLATCH_H */
