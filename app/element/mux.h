#ifndef MUX_H
#define MUX_H

#include "graphicelement.h"

#include <QObject>

class Mux : public GraphicElement {
public:
  explicit Mux( QGraphicsItem *parent = 0 );


  /* GraphicElement interface */
public:
  ElementType elementType( ) {
    return( ElementType::MUX );
  }
  virtual ElementGroup elementGroup( ) {
    return( ElementGroup::MUX );
  }
  void updatePorts();
  void updateLogic( );
};

#endif /* MUX_H */
