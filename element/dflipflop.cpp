#include "dflipflop.h"

#include <QDebug>

DFlipFlop::DFlipFlop(QGraphicsItem * parent) : GraphicElement(4,4,2,2, parent) {
  setPixmap(QPixmap(":/memory/resources/memory/D-flipflop.svg"));
  setRotatable(false);
  updatePorts();
}

DFlipFlop::~DFlipFlop() {

}

void DFlipFlop::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13); //D
  inputs().at(1)->setPos(topPosition(),45); //Clock

  inputs().at(2)->setPos(32,topPosition()); //S
  inputs().at(3)->setPos(32,bottomPosition()); //R

  outputs().at(0)->setPos(bottomPosition(),15); //Q
  outputs().at(1)->setPos(bottomPosition(),45); //Q'
}

void DFlipFlop::updateLogic() {
    char res = outputs().first()->value(); //Output 1
    if(!isValid()) {
      res = -1;
    } else {
      if(inputs().at(1)->value() == true){    //If Clock
        res = inputs().at(0)->value(); //Output = Data
      }
      if (inputs().at(2)->value() == true){
          res = true;
      }
      if (inputs().at(3)->value() == true){
          res = false;
      }
    }
    outputs().first()->setValue(res);
    outputs().last()->setValue(!res);
}
