#ifndef DEMUX_H
#define DEMUX_H

#include "graphicelement.h"

#include <QObject>

class Demux : public GraphicElement {
public:
  explicit Demux( QGraphicsItem *parent = nullptr );
  virtual ~Demux( ) override = default;

  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::MUX );
  }

  /* GraphicElement interface */
public:
  ElementType elementType( ) override {
    return( ElementType::DEMUX );
  }
  void updatePorts( ) override;
  void setSkin( bool defaultSkin, QString filename ) override;
};

#endif /* DEMUX_H */
