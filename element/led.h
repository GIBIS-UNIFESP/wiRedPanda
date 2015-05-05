#ifndef LED_H
#define LED_H

#include "graphicelement.h"

class Led : public GraphicElement {
public:
  explicit Led(QGraphicsItem * parent);
  ~Led();

  // GraphicElement interface
public:
  void updateLogic();
};

#endif // LED_H
