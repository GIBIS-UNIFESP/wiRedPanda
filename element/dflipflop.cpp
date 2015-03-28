#include "dflipflop.h"

#include <QDebug>

DFlipFlop::DFlipFlop(QGraphicsItem * parent) : GraphicElement(4,4,2,2) {
  setPixmap(QPixmap(":/memory/resources/memory/D-flipflop.svg"));
  setRotatable(false);
  updatePorts();
}

DFlipFlop::~DFlipFlop() {

}

void DFlipFlop::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13);
  inputs().at(1)->setPos(topPosition(),45);

  inputs().at(2)->setPos(32,topPosition());
  inputs().at(3)->setPos(32,bottomPosition());

  outputs().at(0)->setPos(bottomPosition(),15);
  outputs().at(1)->setPos(bottomPosition(),45);
}
