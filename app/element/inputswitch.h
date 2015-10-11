#ifndef INPUTSWITCH_H
#define INPUTSWITCH_H

#include "graphicelement.h"

class InputSwitch : public GraphicElement {
public:
  explicit InputSwitch(QGraphicsItem * parent);
  virtual ~InputSwitch();

  bool on;
  // QGraphicsItem interface
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * event);

  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::SWITCH;
  }
  void updateLogic();


  // GraphicElement interface
public:
  void save(QDataStream & ds);
  void load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap, double version);
};

#endif // INPUTSWITCH_H
