#ifndef SRFLIPFLOP_H
#define SRFLIPFLOP_H

#include "graphicelement.h"

class SRFlipFlop : public GraphicElement {
  bool lastClk;

public:
  explicit SRFlipFlop( QGraphicsItem *parent = nullptr );
  virtual ~SRFlipFlop( ) override = default;

  /* GraphicElement interface */
public:
  virtual ElementType elementType( ) override {
    return( ElementType::SRFLIPFLOP );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::MEMORY );
  }
  virtual void updatePorts( ) override;
  void setSkin( bool defaultSkin, QString filename ) override;
};

#endif /* SRFLIPFLOP_H */
