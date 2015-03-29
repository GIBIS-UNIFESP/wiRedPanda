#include "and.h"

And::And(QGraphicsItem * parent) : GraphicElement(2,8,1,1,parent) {
  setOutputsOnTop(true);
  setPixmap(QPixmap(":/basic/resources/basic/and.svg"));
  updatePorts();
}

And::~And() {

}

void And::updateLogic() {
  foreach (QNEPort * input, inputs()) {

  }
}
