#include "inputvcc.h"

InputVcc::InputVcc(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setPixmap(QPixmap(":/input/1.png"));
  setRotatable(false);
  setObjectName("VCC");
  updateLogic();
}

InputVcc::~InputVcc() {

}

void InputVcc::updateLogic() {
  outputs().first()->setValue(true);
}

