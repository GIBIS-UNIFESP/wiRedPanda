#include "not.h"

Not::Not(QGraphicsItem *parent) : GraphicElement(1,1,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/not.png"));
  updatePorts();
}

Not::~Not() {

}

void Not::updateLogic() {
  char res = ! inputs().first()->value();
  if(!isValid()) {
    res = -1;
  }
  outputs().first()->setValue(res);
}


