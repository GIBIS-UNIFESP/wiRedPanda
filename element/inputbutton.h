#ifndef INPUT_H
#define INPUT_H

#include "graphicelement.h"

class InputButton : public GraphicElement {
public:
  explicit InputButton(QGraphicsItem * parent);
  ~InputButton();
  bool on;

  // QGraphicsItem interface
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

  // GraphicElement interface
  public:
  void updateLogic();
};

#endif // INPUT_H
