#include "inputvcc.h"

InputVcc::InputVcc(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setPixmap(QPixmap(":/input/resources/input/1.svg"));
  setRotatable(false);
}

InputVcc::~InputVcc() {

}

void InputVcc::updateLogic() {
  outputs().first()->setValue(true);
}

