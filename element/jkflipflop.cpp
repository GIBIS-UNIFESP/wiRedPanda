#include "jkflipflop.h"

#include <QDebug>

JKFlipFlop::JKFlipFlop(QGraphicsItem * parent) : GraphicElement(5,5,2,2,parent) {
  setPixmap(QPixmap(":/memory/resources/memory/JK-flipflop.svg"));
  setRotatable(false);
  updatePorts();
  lastClk == false;
}

JKFlipFlop::~JKFlipFlop() {

}

void JKFlipFlop::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13); //J
  inputs().at(1)->setPos(topPosition(),29); //Clk
  inputs().at(2)->setPos(topPosition(),45); //K
  inputs().at(3)->setPos(32,topPosition()); //Set
  inputs().at(4)->setPos(32,bottomPosition()); //Reset

  inputs().at(0)->setRequired(false);
  inputs().at(2)->setRequired(false);
  inputs().at(3)->setRequired(false);
  inputs().at(4)->setRequired(false);

  outputs().at(0)->setPos(bottomPosition(),15); //Q
  outputs().at(1)->setPos(bottomPosition(),45); //~Q
}

void JKFlipFlop::updateLogic() {
  char res = qMax( (int) outputs().first()->value(), 0 ); //Output 1
  if(!isValid()) {
    res = -1;
    qDebug() << "Invalid port.";
  } else {
    bool j = inputs().at(2)->value();
    bool k = inputs().at(3)->value();

    if(lastClk == false && inputs().at(1)->value() == true) {   //If Clock
      //JK Logic
      if(inputs().at(2)->value() == inputs().at(3)->value()) { //IF J=K
        res = !res;
      } else if ( j && !k ) { //J
        res = true;
      } else if ( !j && k) { //K
        res = false;
      }
    }
    if (inputs().at(2)->value() == true) { //Set
      res = true;
    }
    if (inputs().at(3)->value() == true) { //Reset
      res = false;
    }
  }
  outputs().first()->setValue(res);
  outputs().last()->setValue(!res);
  lastClk = inputs().at(1)->value();
}
