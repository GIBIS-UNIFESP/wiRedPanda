#include "and.h"

And::And(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/and.png"));
  updatePorts();
}

And::~And() {

}

void And::updateLogic() {
  char res = true;
  if(!isValid()) {
    res = -1;
  } else {
    foreach (QNEPort * input, inputs()) {
      if(input->value() != true) {
        res = false;
        break;
      }
    }
  }
  outputs().first()->setValue(res);
}
