#include "inputgnd.h"

InputGnd::InputGnd(QGraphicsItem * parent) : GraphicElement(0,0,1,1,parent) {
  setOutputsOnTop(false);
  setPixmap(QPixmap(":/input/resources/input/0.svg"));
  setRotatable(false);
}


InputGnd::~InputGnd() {

}

void InputGnd::updateLogic() {
  outputs().first()->setValue(false);
}

