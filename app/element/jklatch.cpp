#include "jklatch.h"

JKLatch::JKLatch(QGraphicsItem * parent) : GraphicElement(2,2,2,2, parent) {
  setPixmap(QPixmap(":/memory/JK-latch.png"));
  setRotatable(false);
  updatePorts();
  setObjectName("JK Latch");
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
  char res = outputs().first()->value();
  char j = inputs().at(0)->value();
  char k = inputs().at(1)->value();
  if(!isValid()) {
    res = -1;
  } else {
    if( j == true &&  k == true) { //IF J=K
      res = !res;
    } else if (j != k) { //J
      res = j;
    } //else nothing happens
  }
  outputs().first()->setValue(res);
  outputs().last()->setValue(!res);
}

//Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#Gated_latches_and_conditional_transparency
