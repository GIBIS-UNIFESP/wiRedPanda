#ifndef JKLATCH_H
#define JKLATCH_H

#include "graphicelement.h"

class JKLatch : public GraphicElement {
public:
  explicit JKLatch( QGraphicsItem *parent = 0 );
  virtual ~JKLatch( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::JKLATCH );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( );
  void updateLogic( );
};

#endif /* JKLATCH_H */
