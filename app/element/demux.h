#ifndef DEMUX_H
#define DEMUX_H

#include "graphicelement.h"

#include <QObject>

class Demux : public GraphicElement {
public:
  explicit Demux( QGraphicsItem *parent = nullptr );
  virtual ElementGroup elementGroup( ) const override {
    return( ElementGroup::MUX );
  }

  /* GraphicElement interface */
public:
  ElementType elementType( ) const override {
    return( ElementType::DEMUX );
  }
  void updatePorts( )override;
};

#endif /* DEMUX_H */
