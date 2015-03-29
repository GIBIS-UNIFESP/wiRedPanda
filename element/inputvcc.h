#ifndef INPUTVCC_H
#define INPUTVCC_H

#include "graphicelement.h"

class InputVcc : public GraphicElement {
public:
  explicit InputVcc(QGraphicsItem * parent);
  ~InputVcc();
  // GraphicElement interface
public:
  void updateLogic();
};

#endif // INPUTVCC_H

