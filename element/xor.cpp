#include "xor.h"

Xor::Xor(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/resources/basic/xor.svg"));
  updatePorts();
}

Xor::~Xor() {

}

