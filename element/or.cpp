#include "or.h"

Or::Or(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/resources/basic/or.svg"));
  updatePorts();
}

Or::~Or() {

}

