#ifndef DEMUX_H
#define DEMUX_H

#include <QObject>
#include <graphicelement.h>

class Demux : public GraphicElement {
public:
  explicit Demux( QGraphicsItem *parent = 0 );
    virtual ElementGroup elementGroup( ) {
      return( ElementGroup::MUX );
    }

  /* GraphicElement interface */
public:
  ElementType elementType( ) {
    return( ElementType::DEMUX );
  }
  void updatePorts();
  void updateLogic( );
};

#endif /* DEMUX_H */
