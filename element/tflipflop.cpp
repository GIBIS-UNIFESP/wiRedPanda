#include "tflipflop.h"

TFlipFlop::TFlipFlop(QGraphicsItem * parent) : GraphicElement(5,5,2,2,parent) {
    setPixmap(QPixmap(":/memory/resources/memory/T-flipflop.svg"));
    setRotatable(false);
    updatePorts();
  }

  TFlipFlop::~TFlipFlop() {

  }

  void TFlipFlop::updatePorts() {
    inputs().at(0)->setPos(topPosition(),13);
    inputs().at(1)->setPos(topPosition(),29);
    inputs().at(2)->setPos(topPosition(),45);
    inputs().at(3)->setPos(32,topPosition());
    inputs().at(4)->setPos(32,bottomPosition());

    outputs().at(0)->setPos(bottomPosition(),15);
    outputs().at(1)->setPos(bottomPosition(),45);
  }

  void TFlipFlop::updateLogic(){
      char res = true;
      if (!isValid()) {
          res = -1;
      } else {
          if(inputs().at(1)->value() == true){    //If Enable
            if (inputs().at(0)->value() == true){ //And T
                res = !(outputs().at(0)->value());
            }
            if (inputs().at(2)->value() == true){ //Set
                res = true;
            }
            if (inputs().at(3)->value() == true){ //Reset
                res = false;
            }
          }
      }
      outputs().first()->setValue(res);
      outputs().last()->setValue(!res);
  }
