#ifndef LED_H
#define LED_H

#include "graphicelement.h"

class Led : public GraphicElement {
public:
  explicit Led(QGraphicsItem * parent);
  ~Led();
protected:

};

#endif // LED_H
