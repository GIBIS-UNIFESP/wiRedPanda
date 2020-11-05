#ifndef DISPLAY14_H
#define DISPLAY14_H

#include "graphicelement.h"

class Display14 : public GraphicElement {
public:
  explicit Display14( QGraphicsItem *parent = nullptr );
  virtual ~Display14( ) override = default;
  static int current_id_number; // Number used to create distinct labels for each instance of this element.

public:
  virtual ElementType elementType( ) override {
    return( ElementType::DISPLAY14 );
  }
  virtual ElementGroup elementGroup( ) override {
    return( ElementGroup::OUTPUT );
  }
  virtual void refresh( ) override;
  void updatePorts( ) override;
  QPixmap bkg, a, b, c, d, e, f, g1, g2, h, j, k, l, m, n, dp;

  /* QGraphicsItem interface */
public:
  void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) override;

  /* GraphicElement interface */
public:
  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) override;
};
#endif /* DISPLAY14_H */