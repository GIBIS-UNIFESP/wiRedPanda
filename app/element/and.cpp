#include "and.h"

And::And(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(":/basic/and.png");
  updatePorts();
  setPortName("AND");
  setToolTip("AND");
}

void And::updateLogic() {
  char res = true;
  if(!isValid()) {
    res = -1;
  } else {
    for (QNEPort * input: inputs()) {
      if(input->value() != true) {
        res = false;
        break;
      }
    }
  }
  outputs().first()->setValue(res);
}
