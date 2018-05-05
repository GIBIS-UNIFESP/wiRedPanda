#ifndef MUX_H
#define MUX_H

#include "graphicelement.h"

#include <QObject>

class Mux : public GraphicElement {
public:
  explicit Mux( QGraphicsItem *parent = nullptr );


  /* GraphicElement interface */
public:
  ElementType elementType( ) const override {
    return( ElementType::MUX );
  }
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::MUX );
  }
  void updatePorts()override;
  void updateLogic( );
};

#endif /* MUX_H */
