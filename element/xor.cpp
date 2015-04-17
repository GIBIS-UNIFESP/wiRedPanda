#include "xor.h"

Xor::Xor(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/resources/basic/xor.svg"));
  updatePorts();
}

Xor::~Xor() {

}

void Xnor::updateLogic() {
  char res = true;
  if(!isValid()) {
    res = -1;
  } else {
    res = 0;
    foreach (QNEPort * input, inputs()) {
      if(input->value() != true) {
        res++;
        break;
      }
    }
  }
  if (res%2==0 || res==0)
      res = false;
  else
      res = true;
  outputs().first()->setValue(res);
}

