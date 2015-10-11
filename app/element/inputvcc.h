#ifndef INPUTVCC_H
#define INPUTVCC_H

#include "graphicelement.h"

class InputVcc : public GraphicElement {
public:
  explicit InputVcc(QGraphicsItem * parent);
  virtual ~InputVcc();
  // GraphicElement interface
public:
  virtual ElementType elementType() {
    return ElementType::VCC;
  }
  void updateLogic();
};

#endif // INPUTVCC_H

