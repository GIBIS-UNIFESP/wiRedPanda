#include "inputvcc.h"

InputVcc::InputVcc(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setPixmap( ":/input/1.png" );
  setRotatable(false);
  setPortName("VCC");
  updateLogic();
}

void InputVcc::updateLogic() {
  outputs().first()->setValue(true);
}

