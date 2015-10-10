#include "or.h"

Or::Or(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/or.png"));
  updatePorts();
}

Or::~Or() {

}

void Or::updateLogic() {
  char res = false;

  if(!isValid()) {
    res = -1;
  } else {
    foreach (QNEPort * input, inputs()) {
      if(input->value() == true) {
        res = true;
        break;
      }
    }
  }
  outputs().first()->setValue(res);
}
