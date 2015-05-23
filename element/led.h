#ifndef LED_H
#define LED_H

#include "graphicelement.h"

class Led : public GraphicElement {
public:
  explicit Led(QGraphicsItem * parent);
  ~Led();

  // GraphicElement interface
  virtual ElementType elementType() {
    return ElementType::LED;
  }
  void updateLogic();
  void setColor(QString color);
  QString color();

private:
  QString m_color;

};

#endif // LED_H
