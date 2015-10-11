#ifndef LED_H
#define LED_H

#include "graphicelement.h"

class Led : public GraphicElement {
public:
  explicit Led(QGraphicsItem * parent);
  virtual ~Led();

  // GraphicElement interface
  virtual ElementType elementType() {
    return ElementType::LED;
  }
  void updateLogic();
  void setColor(QString color);
  QString color();

private:
  QString m_color;


  // GraphicElement interface
public:
  void save(QDataStream & ds);
  void load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap, double version);
};

#endif // LED_H
