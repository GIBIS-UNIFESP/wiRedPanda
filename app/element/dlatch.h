#ifndef DLATCH_H
#define DLATCH_H

#include <graphicelement.h>

class DLatch : public GraphicElement {
public:
  explicit DLatch( QGraphicsItem *parent = 0 );
  virtual ~DLatch( );

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) {
    return( ElementType::DLATCH );
  }
  virtual void updatePorts( );
  void updateLogic( );
};

#endif /* DLATCH_H */
