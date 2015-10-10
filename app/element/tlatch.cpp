#include "tlatch.h"

TLatch::TLatch(QGraphicsItem * parent) : GraphicElement(2,2,2,2,parent) {
  setPixmap(QPixmap(":/memory/T-latch.png"));
  setRotatable(false);
  updatePorts();

}

TLatch::~TLatch() {

}

void TLatch::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13); //T
  inputs().at(1)->setPos(topPosition(),45); //Clock

  outputs().at(0)->setPos(bottomPosition(),15); //Q
  outputs().at(1)->setPos(bottomPosition(),45); //!Q
}

void TLatch::updateLogic(){
    char res = true;
    if(!isValid()) {
      res = -1;
    } else {
      if(inputs().at(1)->value() == true){    //If Enable
        if (inputs().at(0)->value() == true){ //And T
            res = !(outputs().at(0)->value());
        }
      }
    }
    outputs().first()->setValue(res);
    outputs().last()->setValue(!res);
}

