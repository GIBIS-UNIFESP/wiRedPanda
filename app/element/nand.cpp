#include "nand.h"

Nand::Nand(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/resources/basic/nand.svg"));
  updatePorts();
}

Nand::~Nand() {

}

void Nand::updateLogic() {
  char res = false;
  if(!isValid()) {
    res = -1;
  } else {
    foreach (QNEPort * input, inputs()) {
      if(input->value() == false) {
        res = true;
        break;
      }
    }
  }
  outputs().first()->setValue(res);
}

