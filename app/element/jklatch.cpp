#include "jklatch.h"

JKLatch::JKLatch(QGraphicsItem * parent) : GraphicElement(2,2,2,2, parent) {
  setPixmap(QPixmap(":/memory/JK-latch.png"));
  setRotatable(false);
  updatePorts();
}

JKLatch::~JKLatch() {

}

void JKLatch::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13);
  inputs().at(1)->setPos(topPosition(),45);

  outputs().at(0)->setPos(bottomPosition(),15);
  outputs().at(1)->setPos(bottomPosition(),45);
}

void JKLatch::updateLogic() {
  char res = outputs().first()->value(); //Output 1
  if(!isValid()) {
    res = -1;
  } else {
    if(inputs().at(2)->value() == inputs().at(3)->value()) { //IF J=K
      res = !res;
    } else if (inputs().at(2)->value() == true) { //J
      res = true;
    } else if (inputs().at(3)->value() == true) { //K
      res = false;
    }
  }
  outputs().first()->setValue(res);
  outputs().last()->setValue(!res);
}
