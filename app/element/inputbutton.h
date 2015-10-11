#ifndef INPUT_H
#define INPUT_H

#include "graphicelement.h"

class InputButton : public GraphicElement {
public:
  explicit InputButton(QGraphicsItem * parent);
  virtual ~InputButton();
  bool on;

  // QGraphicsItem interface
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

  // GraphicElement interface
  public:
  virtual ElementType elementType() {
    return ElementType::BUTTON;
  }
  void updateLogic();
};

#endif // INPUT_H
