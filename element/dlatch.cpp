#include "dlatch.h"

DLatch::DLatch(QGraphicsItem * parent) : GraphicElement(2,2,2,2,parent) {
  setPixmap(QPixmap(":/memory/resources/memory/D-latch.svg"));
  setRotatable(false);
  updatePorts();

}

DLatch::~DLatch() {

}

void DLatch::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13); //D
  inputs().at(1)->setPos(topPosition(),45); //Enable

  outputs().at(0)->setPos(bottomPosition(),15);
  outputs().at(1)->setPos(bottomPosition(),45);
}

void DLatch::updateLogic(){
    char res = outputs().first()->value(); //Output 1
    if(!isValid()) {
      res = -1;
    } else {
        if(inputs().at(1)->value() == true){    //If Enabled
            res = inputs().at(0)->value(); //Output = Data
        }
    }
    outputs().first()->setValue(res);
    outputs().last()->setValue(!res);
}
