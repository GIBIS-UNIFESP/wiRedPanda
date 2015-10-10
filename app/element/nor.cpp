#include "nor.h"

Nor::Nor(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/nor.png"));
  updatePorts();
  setObjectName("NOR");
}

Nor::~Nor() {

}



void Nor::updateLogic() {
  char res = true;

  if(!isValid()) {
    res = -1;
  } else {
    foreach (QNEPort * input, inputs()) {
      if(input->value() == true) {
        res = false;
        break;
      }
    }
  }
  outputs().first()->setValue(res);
}
