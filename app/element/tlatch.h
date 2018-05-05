#ifndef TLATCH_H
#define TLATCH_H

#include "graphicelement.h"

class TLatch : public GraphicElement {
public:
  explicit TLatch( QGraphicsItem *parent = nullptr );
  virtual ~TLatch( )override;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) const override{
    return( ElementType::TLATCH );
  }
  virtual ElementGroup elementGroup( ) const override{
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( )override;
  void updateLogic( );
};

#endif /* TLATCH_H */
