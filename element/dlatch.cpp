#include "dlatch.h"

DLatch::DLatch(QGraphicsItem * parent) : GraphicElement(2,2,2,2) {
  setPixmap(QPixmap(":/memory/resources/memory/D-latch.svg"));
  setRotatable(false);
  updatePorts();

}

DLatch::~DLatch() {

}

void DLatch::updatePorts() {
  inputs().at(0)->setPos(topPosition(),13);
  inputs().at(1)->setPos(topPosition(),45);

  outputs().at(0)->setPos(bottomPosition(),15);
  outputs().at(1)->setPos(bottomPosition(),45);
}

